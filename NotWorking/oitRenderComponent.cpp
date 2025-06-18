#include "../../../include/odfaeg/Graphics/oitRenderComponent.h"
#include "glCheck.h"
#include <memory.h>
using namespace sf;
using namespace std;
namespace odfaeg {
    namespace graphic {
        OITRenderComponent::OITRenderComponent (RenderWindow& window, int layer, std::string expression, bool twoLayers, sf::ContextSettings settings) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x, window.getView().getPosition().y, layer),
                          math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0),
                          math::Vec3f(window.getView().getSize().x + window.getView().getSize().x * 0.5f, window.getView().getPosition().y + window.getView().getSize().y * 0.5f, layer)),
            view(window.getView()),
            expression(expression),
            twoLayers(twoLayers) {
            update = false;
            sf::Vector3i resolution ((int) window.getSize().x, (int) window.getSize().y, window.getView().getSize().z);
            settings.depthBits = 0;
            frameBuffer.create(resolution.x, resolution.y,settings);
            previousFrame.create(resolution.x, resolution.y, settings);
            settings.depthBits = 24;
            frontBuffer.create(resolution.x, resolution.y,settings);
            depthBuffer.create(resolution.x, resolution.y,settings);
            //frameBuffer.setAlphaTestEnabled(false);
            frontBuffer.setView(window.getView());
            depthBuffer.setView(window.getView());
            frameBuffer.setView(window.getView());
            previousFrame.setView(window.getView());
            //averageBlendEqPart2.setAlphaTestEnabled(false);
            frontBufferSprite = Sprite (frontBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            depthBufferSprite = Sprite (depthBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            frameBufferSprite = Sprite(frameBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            previousFrameBufferSprite = Sprite(previousFrame.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(view.getSize().x, view.getSize().y, 0), sf::IntRect(0, 0, view.getSize().x, view.getSize().y));
            core::FastDelegate<bool> signal (&OITRenderComponent::needToUpdate, this);
            core::FastDelegate<void> slot (&OITRenderComponent::drawNextFrame, this);
            core::Command cmd(signal, slot);
            getListener().connect("UPDATE", cmd);
            viewUpdated = true;
            firstDraw = true;
            secondDraw = false;
            animationUpdated = false;
            previousView = view;
            //if (Shader::isAvailable()) {
                const std::string  vertexShader =
                R"(#version 130
                out mat4 projMat;
                void main () {
                    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
                    gl_FrontColor = gl_Color;
                    projMat = gl_ProjectionMatrix;
                })";
                const std::string  simpleVertexShader =
                R"(#version 130
                void main () {
                    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
                    gl_FrontColor = gl_Color;
                })";
                const std::string transfFragVertexShader =
                R"(#version 130
                out mat4 projMat;
                out mat4 newViewMatrix;
                void main() {
                    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
                    gl_FrontColor = gl_Color;
                    projMat = gl_ProjectionMatrix;
                    newViewMatrix = gl_ModelViewMatrix;
                })";
                const std::string transfFragFragmentShader =
                R"(#version 130
                uniform mat4 oldViewMatrix;
                uniform mat4 viewportMatrix;
                in mat4 projMat;
                in mat4 newViewMatrix;
                uniform vec3 resolution;
                uniform sampler2D oldFrameBuffer;
                uniform vec2 uoffsets;
                void main() {
                    vec4 ndc_coords = inverse(viewportMatrix) * gl_FragCoord;
                    vec4 clip_coords = inverse(projMat) * ndc_coords;
                    clip_coords /= clip_coords.w;
                    vec4 eyes_coords = inverse(newViewMatrix) * clip_coords;
                    eyes_coords = oldViewMatrix * eyes_coords;
                    clip_coords = projMat * eyes_coords;
                    clip_coords /= clip_coords.w;
                    vec4 old_FragCoord = viewportMatrix * clip_coords;
                    /*Doesn't work : it didn't get me the pixel of the previous frame at the previous fragment position.*/
                    vec2 position = (vec2(gl_FragCoord.x + uoffsets.x, gl_FragCoord.y - uoffsets.y) / resolution.xy);
                    vec4 colors[2];
                    colors[0] = vec4(0, 0, 0, 0);
                    colors[1] = texture2D(oldFrameBuffer, position);
                    bool b = (position.x >= 0 && position.y >= 0 && position.x <= resolution.x && position.y <= resolution.y);
                    gl_FragColor = colors[int(b)];
                })";
                const std::string simpleFragShader =
                R"(#version 130
                uniform float haveTexture;
                uniform sampler2D texture;
                void main() {
                    vec4 texel = texture2D(texture, gl_TexCoord[0].xy);
                    vec4 colors[2];
                    colors[0] = gl_Color;
                    colors[1] = texel * gl_Color;
                    bool b = (haveTexture > 0.9);
                    gl_FragColor = colors[int(b)];
                })";
                const std::string depthGenFragShader =
                R"(#version 130
                in mat4 projMat;
                uniform sampler2D texture;
                uniform float haveTexture;
                void main () {
                    vec4 texel = texture2D(texture, gl_TexCoord[0].xy);
                    vec4 colors[2];
                    colors[1] = texel * gl_Color;
                    colors[0] = gl_Color;
                    bool b = (haveTexture > 0.9);
                    vec4 color = colors[int(b)];
                    float z = (gl_FragCoord.w != 1.f) ? (inverse(projMat) * vec4(0, 0, 0, gl_FragCoord.w)).w : gl_FragCoord.z;
                    gl_FragColor = vec4(0, 0, z, color.a);
                })";
                const std::string frameBufferGenFragShader =
                R"(#version 130
                in mat4 projMat;
                uniform sampler2D texture;
                uniform float haveTexture;
                uniform vec3 resolution;
                uniform sampler2D depthBuffer;
                void main () {
                    vec2 position = ( gl_FragCoord.xy / resolution.xy );
                    float max_z = texture2D(depthBuffer, position).z;
                    vec4 texel =  texture2D(texture, gl_TexCoord[0].xy);
                    vec4 colors[2];
                    colors[1] = texel * gl_Color;
                    colors[0] = gl_Color;
                    bool b = (haveTexture > 0.9);
                    vec4 color = colors[int(b)];
                    float z = (gl_FragCoord.w != 1.f) ? (inverse(projMat) * vec4(0, 0, 0, gl_FragCoord.w)).w : gl_FragCoord.z;
                    colors[1] = color;
                    colors[0] = vec4(0, 0, 0, 0);
                    float epsilon = 0.0000000000000000000000000000001;
                    b = ((z + epsilon) < max_z || (z + epsilon) < max_z);
                    gl_FragColor = colors[int(b)];
                })";
                const std::string depthPeelingGenFragShader =
                R"(#version 130
                in mat4 projMat;
                uniform sampler2D depthTexture;
                void main () {
                    vec4 depth = texture2D(depthTexture, gl_TexCoord[0].xy);
                    float z = (gl_FragCoord.w != 1.f) ? (inverse(projMat) * vec4(0, 0, 0, gl_FragCoord.w)).w : gl_FragCoord.z;
                    vec4 colors[2];
                    colors[0] = vec4(0, 0, 0, 0);
                    colors[1] = vec4(0, 0, z, depth.a);
                    float epsilon = 0.0000000000000000000000000000001;
                    bool b = ((z + epsilon) < depth.z || (z + epsilon) < depth.z);
                    gl_FragColor = colors[int(b)];
                })";
                if (!depthBufferGenerator.loadFromMemory(vertexShader, depthGenFragShader))
                    throw core::Erreur(50, "Failed to load depth buffer generator shader", 0);
                if (!frameBufferGenerator.loadFromMemory(vertexShader, frameBufferGenFragShader))
                    throw core::Erreur(51, "Failed to load frame buffer generator shader", 0);
                if (!simpleShader.loadFromMemory(vertexShader, frameBufferGenFragShader))
                    throw core::Erreur(52, "Failed to load simple shader", 0);
                if (!depthPeelingGenerator.loadFromMemory(vertexShader,depthPeelingGenFragShader))
                    throw core::Erreur(53, "Failed to load depth peeling shader", 0);
                if (!transformFragShader.loadFromMemory(transfFragVertexShader, transfFragFragmentShader)) {
                    throw core::Erreur(54, "Failed to load transform frag shader");
                }
                //throw core::Erreur(52, "Failed to load specular texture generator shader", 0);
                frameBufferGenerator.setParameter("resolution",resolution.x, resolution.y, resolution.z);
                frameBufferGenerator.setParameter("depthBuffer", depthBuffer.getTexture());
                frameBufferGenerator.setParameter("texture", Shader::CurrentTexture);
                depthBufferGenerator.setParameter("texture", Shader::CurrentTexture);
                depthPeelingGenerator.setParameter("depthTexture", Shader::CurrentTexture);
                simpleShader.setParameter("texture",Shader::CurrentTexture);
                transformFragShader.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                transformFragShader.setParameter("oldFrameBuffer", previousFrame.getTexture());
                //specularTextureGenerator->setParameter("specularTexture",specularTexture->getTexture());
                backgroundColor = sf::Color::Transparent;
                /*GLuint pboId;
                glCheck(glGenBuffers(2, pboIds));
                glCheck(glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[0]));
                glCheck(glBufferData(GL_PIXEL_PACK_BUFFER, view.getSize().x * view.getSize().y * 4, 0, GL_STREAM_READ));
                glCheck(glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[1]));
                glCheck(glBufferData(GL_PIXEL_PACK_BUFFER, view.getSize().x * view.getSize().y * 4, 0, GL_STREAM_READ));
                glCheck(glBindBuffer(GL_PIXEL_PACK_BUFFER, 0));*/
            /*} else {
                throw core::Erreur(55, "Shader not supported!", 0);
            }*/
        }
        void OITRenderComponent::pushEvent(sf::Event event, RenderWindow& rw) {
            if (event.type == sf::Event::Resized && &getWindow() == &rw && isAutoResized()) {
                std::cout<<"recompute size"<<std::endl;
                recomputeSize();
                getListener().pushEvent(event);
                getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x, getView().getViewport().getPosition().y, getView().getViewport().getPosition().z, event.size.width, event.size.height, getView().getViewport().getDepth()));
            }
        }
        bool OITRenderComponent::needToUpdate() {
            return update;
        }
        void OITRenderComponent::changeVisibleEntities(Entity* toRemove, Entity* toAdd, EntityManager* em) {
            bool removed;
            em->removeAnimatedVisibleEntity(toRemove, visibleEntities, view, removed);
            if (removed) {
                em->insertAnimatedVisibleEntity(toAdd, visibleEntities, view);
                loadEntitiesOnComponent(visibleEntities);
                update = true;
                animationUpdated = true;
            }
        }
        std::string OITRenderComponent::getExpression() {
            return expression;
        }
        void OITRenderComponent::setBackgroundColor(sf::Color color) {
            this->backgroundColor = color;
        }
        void OITRenderComponent::clear() {
             frameBuffer.clear(backgroundColor);
        }
        Sprite& OITRenderComponent::getFrameBufferSprite () {
            return frameBufferSprite;
        }
        Sprite& OITRenderComponent::getDepthBufferSprite() {
            return depthBufferSprite;
        }
        Sprite& OITRenderComponent::getFrontBufferSprite() {
            return frontBufferSprite;
        }
        const Texture& OITRenderComponent::getDepthBufferTexture() {
            return depthBuffer.getTexture();
        }
        const Texture& OITRenderComponent::getFrameBufferTexture() {
            return frameBuffer.getTexture();
        }
        const Texture& OITRenderComponent::getFrontBufferTexture() {
            return frontBuffer.getTexture();
        }
        bool OITRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities)
        {
            //if (firstDraw) {
                /*I tried to redraw the previous frame by transforming fragments when the view is updated to compute the
                transparency for new objects (which are in the updated view) only to reduce the
                number of layers when performing the depthpeeling (otherwise it's too slow) but my shader doesn't work.
            */
                m_newInstances.clear();
                physic::BoundingBox bbox = view.getViewVolume();
                std::map<physic::BoundingBox, std::vector<Entity*>> newEntities;
                newEntities.insert(std::make_pair(bbox, vEntities));
                visibleEntities.clear();
                insertNewEntities(newEntities);
                update = true;
            /*} else if (viewUpdated) {
                m_newInstances.clear();
                visibleEntities.clear();
                batcher.clear();
                physic::BoundingBox viewArea = view.getViewVolume();
                viewArea.setSize(viewArea.getSize().x, viewArea.getSize().y + 100, viewArea.getSize().z);
                physic::BoundingBox oldViewArea = previousView.getViewVolume();
                oldViewArea.setSize(oldViewArea.getSize().x, oldViewArea.getSize().y, oldViewArea.getSize().z);
                std::vector<physic::BoundingBox> areas;
                if (viewArea.getPosition().x < oldViewArea.getPosition().x && viewArea.getPosition().y < oldViewArea.getPosition().y) {
                    int a1PX = viewArea.getPosition().x + (oldViewArea.getPosition().x - viewArea.getPosition().x);
                    int a1PY = viewArea.getPosition().y;
                    int a1SX = viewArea.getSize().x - (oldViewArea.getPosition().x - viewArea.getPosition().x);
                    int a1SY = oldViewArea.getPosition().y - viewArea.getPosition().y;
                    int a2PX = viewArea.getPosition().x;
                    int a2PY = viewArea.getPosition().y;
                    int a2SX = oldViewArea.getPosition().x - viewArea.getPosition().x;
                    int a2SY = viewArea.getSize().y;
                    physic::BoundingBox a1 (a1PX, a1PY, viewArea.getPosition().z, a1SX, a1SY, viewArea.getSize().z);
                    physic::BoundingBox a2 (a2PX, a2PY, viewArea.getPosition().z, a2SX, a2SY, viewArea.getSize().z);
                    areas.push_back(a1);
                    areas.push_back(a2);
                } else if (viewArea.getPosition().x == oldViewArea.getPosition().x && viewArea.getPosition().y > oldViewArea.getPosition().y) {
                    int a1PX = viewArea.getPosition().x;
                    int a1PY = oldViewArea.getPosition().y + oldViewArea.getSize().y;
                    int a1SX = viewArea.getSize().x;
                    int a1SY = viewArea.getPosition().y - oldViewArea.getPosition().y;
                    physic::BoundingBox a1 (a1PX, a1PY, viewArea.getPosition().z, a1SX, a1SY, viewArea.getSize().z);
                    //std::cout<<"a1 : "<<a1.getPosition()<<a1.getSize();
                    areas.push_back(a1);
                } else if (viewArea.getPosition().x > oldViewArea.getPosition().x && viewArea.getPosition().y < oldViewArea.getPosition().y) {
                    int a1PX = viewArea.getPosition().x;
                    int a1PY = viewArea.getPosition().y;
                    int a1SX = viewArea.getSize().x - (viewArea.getPosition().x - oldViewArea.getPosition().x);
                    int a1SY = oldViewArea.getPosition().y - viewArea.getPosition().y;
                    int a2PX = viewArea.getPosition().x + viewArea.getSize().x - (view.getPosition().x - oldViewArea.getPosition().x);
                    int a2PY = viewArea.getPosition().y;
                    int a2SX = viewArea.getPosition().x - oldViewArea.getPosition().x;
                    int a2SY = viewArea.getSize().y - (oldViewArea.getPosition().y - viewArea.getPosition().y);
                    physic::BoundingBox a1 (a1PX, a1PY, viewArea.getPosition().z, a1SX, a1SY, viewArea.getSize().z);
                    physic::BoundingBox a2 (a2PX, a2PY, viewArea.getPosition().z, a2SX, a2SY, viewArea.getSize().z);
                    areas.push_back(a1);
                    areas.push_back(a2);
                } else if (viewArea.getPosition().x == oldViewArea.getPosition().x && viewArea.getPosition().y < oldViewArea.getPosition().y) {
                    int a1PX = viewArea.getPosition().x;
                    int a1PY = viewArea.getPosition().y;
                    int a1SX = viewArea.getSize().x;
                    int a1SY = oldViewArea.getPosition().y - viewArea.getPosition().y;
                    physic::BoundingBox a1 (a1PX, a1PY, viewArea.getPosition().z, a1SX, a1SY, viewArea.getSize().z);
                    areas.push_back(a1);
                } else if (viewArea.getPosition().x < oldViewArea.getPosition().x && viewArea.getPosition().y > oldViewArea.getPosition().y) {
                    int a1PX = viewArea.getPosition().x;
                    int a1PY = viewArea.getPosition().y;
                    int a1SX = oldViewArea.getPosition().x - viewArea.getPosition().x;
                    int a1SY = viewArea.getSize().y;
                    int a2PX = oldViewArea.getPosition().x;
                    int a2PY = oldViewArea.getPosition().y + oldViewArea.getSize().y;
                    int a2SX = viewArea.getSize().x - (oldViewArea.getPosition().x - viewArea.getPosition().x);
                    int a2SY = view.getPosition().y - oldViewArea.getPosition().y;
                    physic::BoundingBox a1 (a1PX, a1PY, viewArea.getPosition().z, a1SX, a1SY, viewArea.getSize().z);
                    physic::BoundingBox a2 (a2PX, a2PY, viewArea.getPosition().z, a2SX, a2SY, viewArea.getSize().z);
                    areas.push_back(a1);
                    areas.push_back(a2);
                } else if (viewArea.getPosition().x < oldViewArea.getPosition().x && viewArea.getPosition().y == oldViewArea.getPosition().y) {
                    int a1PX = viewArea.getPosition().x;
                    int a1PY = viewArea.getPosition().y;
                    int a1SX = oldViewArea.getPosition().x - viewArea.getPosition().x;
                    int a1SY = viewArea.getSize().y;
                    physic::BoundingBox a1 (a1PX, a1PY, viewArea.getPosition().z, a1SX, a1SY, viewArea.getSize().z);
                    areas.push_back(a1);
                } else if (viewArea.getPosition().x > oldViewArea.getPosition().x && viewArea.getPosition().y > oldViewArea.getPosition().y) {
                    int a1PX = viewArea.getPosition().x + viewArea.getSize().x - (viewArea.getPosition().x - oldViewArea.getPosition().x);
                    int a1PY = viewArea.getPosition().y;
                    int a1SX = viewArea.getPosition().x - oldViewArea.getPosition().x;
                    int a1SY = viewArea.getSize().y - (viewArea.getPosition().y - oldViewArea.getPosition().y);
                    int a2PX = viewArea.getPosition().x;
                    int a2PY = oldViewArea.getPosition().y - (viewArea.getPosition().y - oldViewArea.getPosition().y);
                    int a2SX = viewArea.getPosition().x;
                    int a2SY = viewArea.getPosition().y - oldViewArea.getPosition().y;
                    physic::BoundingBox a1 (a1PX, a1PY, viewArea.getPosition().z, a1SX, a1SY, viewArea.getSize().z);
                    physic::BoundingBox a2 (a2PX, a2PY, viewArea.getPosition().z, a2SX, a2SY, viewArea.getSize().z);
                    areas.push_back(a1);
                    areas.push_back(a2);
                } else if (viewArea.getPosition().x > oldViewArea.getPosition().x && viewArea.getPosition().y == oldViewArea.getPosition().y) {
                    int a1PX = viewArea.getPosition().x;
                    int a1PY = viewArea.getPosition().y;
                    int a1SX = viewArea.getSize().x - (viewArea.getPosition().x - oldViewArea.getPosition().x);
                    int a1SY = viewArea.getSize().y;
                    physic::BoundingBox a1 (a1PX, a1PY, viewArea.getPosition().z, a1SX, a1SY, viewArea.getSize().z);
                    areas.push_back(a1);
                }
                std::map<physic::BoundingBox, std::vector<Entity*>> newEntities;
                for (unsigned int i = 0; i < areas.size(); i++) {
                    newEntities.insert(std::make_pair(areas[i], std::vector<Entity*>()));
                }
                for (unsigned int i = 0; i < vEntities.size(); i++) {
                    physic::BoundingBox bounds = vEntities[i]->getGlobalBounds();
                    if (bounds.intersects(oldViewArea)) {
                        for (unsigned int j = 0; j < vEntities[i]->getFaces().size(); j++) {
                            batcher.addFace(vEntities[i]->getFaces()[j]);
                        }
                        visibleEntities.push_back(vEntities[i]);
                    }
                    std::map<physic::BoundingBox, std::vector<Entity*>>::iterator it;
                    for (unsigned int j = 0; j < areas.size(); j++) {
                        if (areas[j].intersects(bounds)) {
                            //std::cout<<"intersects"<<std::endl;
                            it = newEntities.find(areas[j]);
                            it->second.push_back(vEntities[i]);
                        }
                    }
                }
                m_instances = batcher.getInstances();
                insertNewEntities(newEntities);
            }*/
            return true;
        }
        void OITRenderComponent::setView(View view) {
            previousView = this->view;
            this->view = view;
            frameBuffer.setView(view);
            depthBuffer.setView(view);
            frontBuffer.setView(view);
            previousFrame.setView(view);
            viewUpdated = true;
        }
        void OITRenderComponent::setExpression(std::string expression) {
            update = true;
            this->expression = expression;
        }
        void OITRenderComponent::drawNextFrame() {
            update = false;
            /*TransformFragShader doesn't work."*/
            /*if (!firstDraw && viewUpdated) {
                transformFragShader.setParameter("oldViewMatrix", previousView.getViewMatrix().getMatrix().transpose());
                ViewportMatrix vpm;
                vpm.setViewport(previousView.getViewport().getPosition(), previousView.getViewport().getSize());
                transformFragShader.setParameter("viewportMatrix", vpm.getMatrix().transpose());
                math::Vec3f offsets = view.getPosition() - previousView.getPosition();
                //Strange things happens at the texture's borders, when the view moves right or top, pixels are not transparent
                //when the pixel position is greater than the screen resolution.
                transformFragShader.setParameter("uoffsets", offsets.x, offsets.y);
                currentStates.blendMode = BlendNone;
                currentStates.shader = &transformFragShader;
                for (unsigned int i = 0; i < m_instances.size(); i++) {
                    physic::BoundingBox bbox = previousView.getViewVolume();
                    math::Vec3f bbScreenPos = frameBuffer.mapCoordsToPixel(bbox.getPosition(), view);
                    frameBuffer.setActive();
                    glCheck(glEnable(GL_SCISSOR_TEST));
                    glCheck(glScissor(bbScreenPos.x, bbox.getSize().y - (bbScreenPos.y + bbox.getSize().y),bbox.getSize().x, bbox.getSize().y));
                    frameBuffer.draw(m_instances[i].getAllVertices(), currentStates);
                    glCheck(glDisable(GL_SCISSOR_TEST));
                }
                frameBuffer.display();
            }*/
            /*Need to draw the scene twice at the very first draw call because at the first call the depthtest (on the depth texture) never pass and it doesn't draw anything.
            this is very strange.*/
           // if (viewUpdated || firstDraw || secondDraw || animationUpdated) {
                depthBuffer.clear(sf::Color::Transparent);
                frontBuffer.clear(sf::Color::Transparent);
                currentStates.blendMode = BlendNone;
                std::map<physic::BoundingBox, std::pair<unsigned int, std::vector<Instance>>>::iterator it;
                for (it = m_newInstances.begin(); it != m_newInstances.end(); it++) {
                    for (unsigned int i = 0; i < it->second.second.size(); i++) {
                         if (it->second.second[i].getMaterial().getTexture() == nullptr) {
                            depthBufferGenerator.setParameter("haveTexture", 0);
                            simpleShader.setParameter("haveTexture", 0);
                         } else {
                            depthBufferGenerator.setParameter("haveTexture", 1);
                            simpleShader.setParameter("haveTexture", 1);
                         }
                         currentStates.shader=&depthBufferGenerator;
                         currentStates.texture=it->second.second[i].getMaterial().getTexture();
                         physic::BoundingBox bbox = it->first;
                         math::Vec3f bbScreenPos = depthBuffer.mapCoordsToPixel(bbox.getPosition(), view);
                         /*depthBuffer.setActive();
                         glCheck(glEnable(GL_SCISSOR_TEST));
                         glCheck(glScissor(bbScreenPos.x, bbox.getSize().y - (bbScreenPos.y + bbox.getSize().y),bbox.getSize().x, bbox.getSize().y));*/
                         depthBuffer.draw(it->second.second[i].getAllVertices(),currentStates);
                         //glCheck(glDisable(GL_SCISSOR_TEST));
                         currentStates.shader = nullptr;
                         /*frontBuffer.setActive();
                         glCheck(glEnable(GL_SCISSOR_TEST));
                         glCheck(glScissor(bbScreenPos.x, bbox.getSize().y - (bbScreenPos.y + bbox.getSize().y),bbox.getSize().x, bbox.getSize().y));*/
                         frontBuffer.draw(it->second.second[i].getAllVertices(), currentStates);
                         //glCheck(glDisable(GL_SCISSOR_TEST));
                    }
                }
                frontBuffer.display();
                depthBuffer.display();
                frontBufferSprite.setCenter(view.getPosition());
                currentStates.blendMode = sf::BlendMode(sf::BlendMode::Factor::OneMinusDstAlpha, sf::BlendMode::Factor::DstAlpha, sf::BlendMode::Add, sf::BlendMode::Factor::OneMinusDstAlpha, sf::BlendMode::Factor::One, sf::BlendMode::Equation::Add);
                frameBuffer.draw(frontBufferSprite, currentStates);
                if (twoLayers) {
                    /*
                        this solution only works with two layers.
                    */
                    currentStates.shader=&frameBufferGenerator;
                    currentStates.blendMode=sf::BlendMode(sf::BlendMode::Factor::OneMinusDstAlpha, sf::BlendMode::Factor::DstAlpha, sf::BlendMode::Equation::Add, sf::BlendMode::Factor::OneMinusDstAlpha, sf::BlendMode::Factor::One, sf::BlendMode::Equation::Add);
                    for (it = m_newInstances.begin(); it != m_newInstances.end(); it++) {
                        for (unsigned int i = 0; i < it->second.second.size(); i++) {
                            if (it->second.second[i].getMaterial().getTexture() == nullptr)
                                frameBufferGenerator.setParameter("haveTexture", 0);
                            else
                                frameBufferGenerator.setParameter("haveTexture", 1);
                            currentStates.texture = it->second.second[i].getMaterial().getTexture();
                            physic::BoundingBox bbox = it->first;
                            math::Vec3f bbScreenPos = frameBuffer.mapCoordsToPixel(bbox.getPosition(), view);
                            /*frameBuffer.setActive();
                            glCheck(glEnable(GL_SCISSOR_TEST));
                            glCheck(glScissor(bbScreenPos.x, bbox.getSize().y - (bbScreenPos.y + bbox.getSize().y),bbox.getSize().x, bbox.getSize().y));*/
                            frameBuffer.draw(it->second.second[i].getAllVertices(), currentStates);
                            //glCheck(glDisable(GL_SCISSOR_TEST));
                        }
                    }
                } else {
                    /*
                        This solution works with n layers but it's too slow, I tried to optimize it to reduce the number of layers
                        by computing transparency only for objects added in the new view but it doensn't work.
                    */
                    /*unsigned int nbLayers = 0;
                    unsigned int size = view.getSize().x * view.getSize().y * 4;
                    const sf::Uint8 * data = new unsigned char[size];
                    depthBuffer.display();
                    sf::Image img = depthBuffer.getTexture().copyToImage();
                    data = img.getPixelsPtr();
                    for (unsigned int i = 0; i < size / 4; i++) {
                        if ((float)(data[i*4+2])  != nbLayers) {
                            nbLayers++;
                        }
                    }*/
                    for (it = m_newInstances.begin(); it != m_newInstances.end(); it++) {
                        unsigned int nbLayers = (it->second.first > 0) ? it->second.first - 1 : 0;
                        std::cout<<"nb layers : "<<nbLayers<<std::endl;
                        for (unsigned int l = 0; l < nbLayers; l++) {

                                for (unsigned int i = 0; i < it->second.second.size(); i++) {
                                    currentStates.shader = &frameBufferGenerator;
                                    currentStates.blendMode = sf::BlendNone;
                                    frontBuffer.clear(sf::Color::Transparent);
                                     if (it->second.second[i].getMaterial().getTexture() == nullptr) {
                                        frameBufferGenerator.setParameter("haveTexture", 0);
                                     } else {
                                        frameBufferGenerator.setParameter("haveTexture", 1);
                                     }
                                     currentStates.texture=it->second.second[i].getMaterial().getTexture();
                                     physic::BoundingBox bbox = it->first;
                                     math::Vec3f bbScreenPos = frontBuffer.mapCoordsToPixel(bbox.getPosition(), view);
                                     /*frontBuffer.setActive();
                                     glCheck(glEnable(GL_SCISSOR_TEST));
                                     glCheck(glScissor(bbScreenPos.x, bbox.getSize().y - (bbScreenPos.y + bbox.getSize().y),bbox.getSize().x, bbox.getSize().y));*/
                                     frontBuffer.draw(it->second.second[i].getAllVertices(), currentStates);
                                     //glCheck(glDisable(GL_SCISSOR_TEST));
                                }
                                frontBuffer.display();
                                frontBufferSprite.setCenter(view.getPosition());
                                Texture tmpDepthText(depthBuffer.getTexture());
                                currentStates.shader = nullptr;
                                currentStates.blendMode = sf::BlendNone;
                                //Problem : it doesn't draw anything on the depthBuffer after he's cleared.
                                depthBuffer.clear(sf::Color::Transparent);
                                currentStates.texture = &tmpDepthText;
                                currentStates.shader = &depthPeelingGenerator;
                                for (unsigned int i = 0; i < it->second.second.size(); i++) {
                                     physic::BoundingBox bbox = it->first;
                                     math::Vec3f bbScreenPos = frontBuffer.mapCoordsToPixel(bbox.getPosition(), view);
                                     /*depthBuffer.setActive();
                                     glCheck(glEnable(GL_SCISSOR_TEST));
                                     glCheck(glScissor(bbScreenPos.x, bbox.getSize().y - (bbScreenPos.y + bbox.getSize().y),bbox.getSize().x, bbox.getSize().y));*/
                                     depthBuffer.draw(it->second.second[i].getAllVertices(), currentStates);
                                     //glCheck(glDisable(GL_SCISSOR_TEST));
                                }
                                depthBuffer.display();
                                currentStates.shader = nullptr;
                                currentStates.blendMode = sf::BlendMode(sf::BlendMode::Factor::OneMinusDstAlpha, sf::BlendMode::Factor::DstAlpha, sf::BlendMode::Add, sf::BlendMode::Factor::OneMinusDstAlpha, sf::BlendMode::Factor::One, sf::BlendMode::Equation::Add);
                                frameBuffer.draw(frontBufferSprite, currentStates);
                        }
                    }
                }
                for (unsigned int i = 0; i < drawables.size(); i++) {
                    frameBuffer.draw(drawables[i].first.get(), drawables[i].second);
                }
                frameBuffer.display();
                /*previousFrame.clear(backgroundColor);
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = nullptr;
                frameBufferSprite.setCenter(view.getPosition());
                previousFrame.draw(frameBufferSprite, currentStates);
                previousFrame.display();
                previousFrameBufferSprite.setCenter(view.getPosition());
                if (firstDraw)
                    secondDraw = true;
                if (!firstDraw)
                    secondDraw = false;
                firstDraw = false;
                viewUpdated = false;
            } else {
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = nullptr;
                frameBuffer.draw(previousFrameBufferSprite, currentStates);
                frameBuffer.display();
            }*/
        }
       std::vector<Entity*> OITRenderComponent::getEntities() {
            return visibleEntities;
        }
        void OITRenderComponent::draw(Drawable& drawable, RenderStates states) {
            update = true;
            drawables.push_back(std::make_pair(std::ref(drawable), states));
        }
        void OITRenderComponent::draw(RenderTarget& target, RenderStates states) {
            frameBufferSprite.setCenter(target.getView().getPosition());
            target.draw(frameBufferSprite, states);
            drawables.clear();
        }

        View& OITRenderComponent::getView() {
            return view;
        }
        int OITRenderComponent::getLayer() {
            return getPosition().z;
        }
        void OITRenderComponent::removeEntities(physic::BoundingBox box) {
            std::vector<Entity*>::iterator it;
            for (it = visibleEntities.begin(); it != visibleEntities.end();) {
                physic::BoundingBox bounds = (*it)->getGlobalBounds();
                if (bounds.intersects(box)) {
                    it = visibleEntities.erase(it);
                } else {
                    it++;
                }
            }
        }
        void OITRenderComponent::insertNewEntities(std::map<physic::BoundingBox, std::vector<Entity*>> vEntities) {
            std::map<physic::BoundingBox, std::vector<Entity*>>::iterator it;
            for (it = vEntities.begin(); it != vEntities.end(); it++) {
                newBatcher.clear();
                for (unsigned int i = 0; i < it->second.size(); i++) {
                    if ( it->second[i]->isLeaf()) {
                        for (unsigned int j = 0; j <  it->second[i]->getFaces().size(); j++) {
                             newBatcher.addFace(it->second[i]->getFaces()[j]);
                        }
                    }
                    visibleEntities.push_back(it->second[i]);
                }
                m_newInstances.insert(std::make_pair(it->first, std::make_pair(newBatcher.getNbLayers(), newBatcher.getInstances())));
            }
            update = true;
        }
        void OITRenderComponent::updateParticleSystems() {
            for (unsigned int i = 0; i < visibleEntities.size(); i++) {
                if (dynamic_cast<physic::ParticleSystem*>(visibleEntities[i]) != nullptr) {
                    static_cast<physic::ParticleSystem*>(visibleEntities[i])->update();
                }
            }
            loadEntitiesOnComponent(visibleEntities);
            update = true;
            animationUpdated = true;
        }
        OITRenderComponent::~OITRenderComponent() {
            //glDeleteBuffers(2, pboIds);
        }
    }
}

module;
#include <cassert>
#include <vector>
#include <functional>
#include <iostream>
#include <ostream>
//import odfaeg.graphic.particleSystem;
module odfaeg.entity.particleSystem;
import odfaeg.entity.rect;
import odfaeg.math.vec;
namespace odfaeg {
    namespace entity {
        namespace
        {
            FloatRect getFullRect()
            {
                return FloatRect(0.f, 0.f, 1.f, 1.f);
            }
        } // namespa
        ParticleSystem::ParticleSystem(math::Vec3f position, math::Vec3f size, math::Vec2f particlesSize) : GameObject(position, size,size*0.5f,"E_PARTICLE_SYSTEM") {
            SubMesh sm;
            addSubMesh(sm);
            textureId = "";
            this->particlesSize = particlesSize;
        }
        void ParticleSystem::setTexture(std::string textureId) {
            this->textureId = textureId;
            getSubMeshes()[0].setTextureId(SubMesh::DIFFUSE, textureId);
        }
        unsigned int ParticleSystem::addTextureRect(const FloatRect& textureRect) {
            textureRects.push_back(textureRect);
            return textureRects.size()-1;
        }
        unsigned int ParticleSystem::computeMaxParticles() {
            unsigned int maxParticulesCount = 0;
            for (unsigned int i = 0; i < universalEmittors.size(); i++) {
                maxParticulesCount += universalEmittors[i].maxParticleLifeTime.asSeconds() * universalEmittors[i].emissionRate;
            }
            return maxParticulesCount;
        }
        std::vector<ParticleSystem::Quad>& ParticleSystem::getQuads() {
            return quads;
        }
        void ParticleSystem::computeQuads()
        {            
            /*int pause;
            std::cin>>pause;*/
            // No texture rects: Use full texture, cache single rectangle
            if (textureRects.empty())
            {
                quads.resize(1);
                computeQuad(quads[0], getFullRect());
            }

            // Specified texture rects: Cache every one
            else
            {

                quads.resize(textureRects.size());
                for (std::size_t i = 0; i < textureRects.size(); ++i)
                    computeQuad(quads[i], textureRects[i]);
            }
        }

        void ParticleSystem::computeQuad(Quad& quad, const FloatRect& textureRect)
        {
            FloatRect rect(textureRect);
            quad[0].texCoords = math::Vec2f(rect.left, rect.top);
            quad[1].texCoords = math::Vec2f(rect.left + rect.width, rect.top);
            quad[2].texCoords = math::Vec2f(rect.left + rect.width, rect.top + rect.height);
            quad[3].texCoords = math::Vec2f(rect.left, rect.top + rect.height);
            float texW = particlesSize.x();
            float texH = particlesSize.y();
            float w = rect.width  * texW;
            float h = rect.height * texH;
            quad[0].position = math::Vec3f(-w, -h, 0.f) / 2.f;
            quad[1].position = math::Vec3f( w, -h, 0.f) / 2.f;
            quad[2].position = math::Vec3f( w, h, 0.f) / 2.f;
            quad[3].position = math::Vec3f(-w, h, 0.f) / 2.f;
            quad[0].normal = math::Vec3f(0.f, 0.f, 1.f);
            quad[1].normal = math::Vec3f(0.f, 0.f, 1.f);
            quad[2].normal = math::Vec3f(0.f, 0.f, 1.f);
            quad[3].normal = math::Vec3f(0.f, 0.f, 1.f);
            /*std::cout<<"quad tex coords : "<<quad[0].texCoords.x()<<","<<quad[0].texCoords.y()<<std::endl;
            std::cout<<"quad tex coords : "<<quad[1].texCoords.x()<<","<<quad[1].texCoords.y()<<std::endl;
            std::cout<<"quad tex coords : "<<quad[2].texCoords.x()<<","<<quad[2].texCoords.y()<<std::endl;
            std::cout<<"quad tex coords : "<<quad[3].texCoords.x()<<","<<quad[3].texCoords.y()<<std::endl;
            int pause;
            std::cin>>pause;*/
        }
        void ParticleSystem::computeVertices() {
            VertexArray& va = getSubMeshes()[0].getVertexArray();
            va.clear();
            va.setPrimitiveType(Triangles);
            for (unsigned int i = 0; i < computeMaxParticles(); i++) {
                for (unsigned int j = 0; j < 4; j++) {
                    Vertex vertex;
                    va.append(vertex);
                }
                va.addIndex(i*4+0);
                va.addIndex(i*4+1);
                va.addIndex(i*4+2);
                va.addIndex(i*4+0);
                va.addIndex(i*4+2);
                va.addIndex(i*4+3);
            }
            getSubMeshes()[0].setVertexArray(va);
        }
        GameObject* ParticleSystem::clone() {
            ParticleSystem* ps = new ParticleSystem(getPosition(), getSize(), particlesSize);
            GameObject::copy(ps);
            ps->universalEmittors = universalEmittors;
            return ps;
        }
    }
}

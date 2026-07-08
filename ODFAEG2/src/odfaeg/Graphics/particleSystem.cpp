module;
#include <cassert>
#include <vector>
#include <functional>
#include <iostream>
#include <ostream>
//import odfaeg.graphic.particleSystem;
module odfaeg.graphic.particleSystem;
import odfaeg.graphic.rect;
import odfaeg.math.vec;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.material;
import odfaeg.graphic.texture;
namespace odfaeg {
    namespace graphic {
        namespace
        {
            FloatRect getFullRect(const Texture& texture)
            {
                return FloatRect(0.f, 0.f, 1.f, 1.f);
            }
        } // namespa
        ParticleSystem::ParticleSystem(math::Vec3f position, math::Vec3f size) : GameObject(position, size,size*0.5f,"E_PARTICLE_SYSTEM") {
            SubMesh sm(GPUContext::instance().getDevice());
            addSubMesh(std::move(sm));
            texture = nullptr;
        }
        ParticleSystem::ParticleSystem(ParticleSystem&& other) noexcept : GameObject(std::move(other)) {
            universalEmittors = other.universalEmittors;
            quads = other.quads;
            textureRects = other.textureRects;
            texture = other.texture;
        }
        void ParticleSystem::setTexture(Texture* texture) {
            this->texture = texture;
            getSubMeshes()[0].getMaterial().setTexture(texture, Material::DIFFUSE);
        }
        unsigned int ParticleSystem::addTextureRect(const graphic::FloatRect& textureRect) {
            textureRects.push_back(textureRect);
            return textureRects.size()-1;
        }
        ParticleSystem& ParticleSystem::operator=(ParticleSystem&& other) noexcept {
            if (this != &other) {
                GameObject::operator=(std::move(other));
                universalEmittors = other.universalEmittors;;
                quads = other.quads;
                textureRects = other.textureRects;
                texture = other.texture;
            }
            return *this;
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
            // Ensure setTexture() has been called
            assert(texture);
            /*int pause;
            std::cin>>pause;*/
            // No texture rects: Use full texture, cache single rectangle
            if (textureRects.empty())
            {
                quads.resize(1);
                computeQuad(quads[0], getFullRect(*texture));
            }

            // Specified texture rects: Cache every one
            else
            {

                quads.resize(textureRects.size());
                for (std::size_t i = 0; i < textureRects.size(); ++i)
                    computeQuad(quads[i], textureRects[i]);
            }
        }

        void ParticleSystem::computeQuad(Quad& quad, const graphic::FloatRect& textureRect)
        {
            FloatRect rect(textureRect);
            quad[0].texCoords = math::Vec2f(rect.left, rect.top);
            quad[1].texCoords = math::Vec2f(rect.left + rect.width, rect.top);
            quad[2].texCoords = math::Vec2f(rect.left + rect.width, rect.top + rect.height);
            quad[3].texCoords = math::Vec2f(rect.left, rect.top + rect.height);
            float texW = texture->getSize().x();
            float texH = texture->getSize().y();
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
            VertexBuffer& vb = getSubMeshes()[0].getVertexBuffer();
            vb.clear();
            vb.setPrimitiveType(Triangles);
            for (unsigned int i = 0; i < computeMaxParticles(); i++) {
                for (unsigned int j = 0; j < 4; j++) {
                    Vertex vertex;
                    vb.append(vertex);
                }
                vb.addIndex(i*4+0);
                vb.addIndex(i*4+1);
                vb.addIndex(i*4+2);
                vb.addIndex(i*4+0);
                vb.addIndex(i*4+2);
                vb.addIndex(i*4+3);
            }
            getSubMeshes()[0].setVertexBuffer(vb);
        }
        GameObject* ParticleSystem::clone() {
            ParticleSystem* ps = new ParticleSystem(getPosition(), getSize());
            GameObject::copy(ps);
            ps->universalEmittors = universalEmittors;
            return ps;
        }
    }
}

#ifndef ODFAEG_PARTICLESYSTEM_HPP
#define ODFAEG_PARTICLESSYSTEM_HPP
#include <cassert>
#include <vector>
#include <functional>
#include <iostream>
#include <ostream>
#include "rect.hpp"
#include "gameObject.hpp"
namespace odfaeg {
    namespace entity {
        namespace
        {
            FloatRect getFullRect()
            {
                return FloatRect(0.f, 0.f, 1.f, 1.f);
            }
        } // namespa
        template <typename T>
        concept Universal = std::is_same_v<T, UniversalEmittor>;
        class ParticleSystem : public GameObject {
        public :
            typedef std::array<Vertex, 4>	Quad;
            ParticleSystem(math::Vec3f position, math::Vec3f size, math::Vec2f particleSize);           
            unsigned int computeMaxParticles();
            void setTexture(std::string textureId);
            unsigned int addTextureRect(const FloatRect& textureRect);
            template <typename Emittor>
            void addEmittor(Emittor emittor) {
                if constexpr (Universal<Emittor>)
                    universalEmittors.push_back(emittor);
            }
            template <typename Emittor>
            void removeEmittor(Emittor& emittor) {
                typename std::vector<Emittor>::iterator itr;
                if constexpr (Universal<Emittor>) {
                    for (itr = universalEmittors.begin(); itr != universalEmittors.end(); itr++) {
                        if (*itr == &emittor) {
                            itr = universalEmittors.erase(itr);
                        } else {
                            itr++;
                        }
                    }
                }
            }
            template<typename Emittor>
            std::vector<Emittor>& getEmittors() {
                if constexpr (Universal<Emittor>)
                    return universalEmittors;
            }
            void computeQuads();
            void computeQuad(Quad& quad, const FloatRect& textureRect);
            std::vector<Quad>& getQuads();
            void computeVertices();
            GameObject* clone();
        private :
            std::vector<UniversalEmittor> universalEmittors;
            std::vector<FloatRect> textureRects;
            std::string textureId;
            std::vector<Quad> quads;
            math::Vec2f particlesSize;
        };
    }
}
#include "particleSystem.inl"
#endif
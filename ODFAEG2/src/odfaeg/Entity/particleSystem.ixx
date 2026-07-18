module;
#include <functional>
export module odfaeg.entity.particleSystem;
import odfaeg.entity.particle;
import odfaeg.entity.emittors;
import odfaeg.core.clock;
import odfaeg.entity.gameObject;
import odfaeg.entity.rect;
import odfaeg.entity.vertex;
import odfaeg.entity.vertexArray;
import odfaeg.math.vec;

namespace odfaeg {
    namespace entity {
        template <typename T>
        concept Universal = std::is_same_v<T, UniversalEmittor>;
        export class ParticleSystem : public GameObject {
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

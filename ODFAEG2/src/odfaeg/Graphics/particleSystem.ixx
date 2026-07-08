module;
#include <functional>
export module odfaeg.graphic.particleSystem;
import odfaeg.graphic.particle;
import odfaeg.graphic.emittors;
import odfaeg.core.clock;
import odfaeg.graphic.gameObject;
import odfaeg.graphic.pipeline;
import odfaeg.graphic.buffer;
import odfaeg.graphic.rect;
import odfaeg.graphic.texture;
import odfaeg.graphic.vertex;
import odfaeg.graphic.vertexBuffer;
import odfaeg.math.vec;

namespace odfaeg {
    namespace graphic {
        template <typename T>
        concept Universal = std::is_same_v<T, UniversalEmittor>;
        export class ParticleSystem : public GameObject {
        public :
            typedef std::array<Vertex, 4>	Quad;
            ParticleSystem(math::Vec3f position, math::Vec3f size);
            ParticleSystem(ParticleSystem&& particleSystem) noexcept;
            ParticleSystem& operator=(ParticleSystem&& particleSystem) noexcept;
            unsigned int computeMaxParticles();
            void setTexture(Texture* texture);
            unsigned int addTextureRect(const graphic::FloatRect& textureRect);
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
            void computeQuad(Quad& quad, const graphic::FloatRect& textureRect);
            std::vector<Quad>& getQuads();
            void computeVertices();
            GameObject* clone();
        private :
            std::vector<UniversalEmittor> universalEmittors;
            std::vector<FloatRect> textureRects;
            Texture* texture;
            std::vector<Quad> quads;
        };
    }
}

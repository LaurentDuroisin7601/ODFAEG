export module odfaeg.entity.particle;
import odfaeg.math.vec;
import odfaeg.entity.color;
import odfaeg.core.clock;
namespace odfaeg {
    namespace entity {
        export struct alignas(16) Particle {
            alignas(16) math::Vec3f position;
            float pad1;
            alignas(16) math::Vec3f velocity;
            float pad2;
            alignas(16) math::Vec3f scale;
            float pad3;
            Color color;
            unsigned int id, textureId;
            float rotation;
            float rotationSpeed;
            core::Time passedLifeTime, totalLifeTime;
            unsigned int alive;
            unsigned int particleSystemId;
        };
    }
}
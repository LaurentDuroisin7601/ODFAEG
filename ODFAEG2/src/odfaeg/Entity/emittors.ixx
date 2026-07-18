module;
#include <functional>
#include <cstdint>
export module odfaeg.entity.emittors;
import odfaeg.core.clock;
import odfaeg.math.vec;
import odfaeg.entity.color;
namespace odfaeg {
    namespace entity {
        export std::uint32_t packColor(Color color) {
            return (color.a << 24) | (color.b << 16) | (color.g << 8) | color.r;
        }
        export struct alignas(16) EmittorInterface {
            core::Time timeUntilRemoval=core::seconds(0);
            unsigned int particleSystemId=0, alive=1;
            float emissionRate, emissionDifference=0;
        };
        export struct alignas(16) UniversalEmittor : public EmittorInterface {
            core::Time minParticleLifeTime, maxParticleLifeTime;
            alignas(16) math::Vec3f minParticlePosition;
            //float pad1;
            alignas(16) math::Vec3f maxParticlePosition;
            //float pad2;
            alignas(16) math::Vec3f minParticleVelocity;
            //float pad3;
            alignas(16) math::Vec2f maxParticleVelocity;
            //float pad4;
            float minParticleRotation, maxParticleRotation;
            float minParticleRotationSpeed, maxParticleRotationSpeed;
            alignas(16) math::Vec3f minParticleScale;
            //float pad5;
            alignas(16) math::Vec3f maxParticleScale;
            //float pad6;
            std::uint32_t minParticleColor, maxParticleColor;
            unsigned int minParticleTextureIndex, maxParticleTextureIndex;
        };
    }
}

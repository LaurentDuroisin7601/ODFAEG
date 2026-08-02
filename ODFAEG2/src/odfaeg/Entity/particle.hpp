#ifndef ODFAEG_PARTICLE_HPP
#define ODFAEG_PARTICLE_HPP
namespace odfaeg {
    namespace entity {
        struct alignas(16) Particle {
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
#endif
#include "../../../include/odfaeg/Graphics/particleSystemUpdater.hpp"
namespace odfaeg {
    namespace graphic {
        ParticleSystemUpdater::ParticleSystemUpdater(bool useThread) : Timer(useThread) {}
        /**
        * \fn void onUpdate ()
        * \brief update all the entities which are in the current view.
        */
        void ParticleSystemUpdater::addParticleSystem(Entity* ps) {
            ps->setPsUpdater(getName());
            particleSystems.push_back(ps);
        }
        void ParticleSystemUpdater::removeParticleSystem (Entity* ps) {
            std::vector<Entity*>::iterator it;
            for (it = particleSystems.begin(); it != particleSystems.end();) {
                if (*it == ps) {
                    it = particleSystems.erase(it);
                } else {
                    it++;
                }
            }
        }
        std::vector<Entity*> ParticleSystemUpdater::getParticleSystems() {
            return particleSystems;
        }
        void ParticleSystemUpdater::onUpdate () {
            for (unsigned int i = 0; i < particleSystems.size(); i++) {
                ////////std::cout<<"update particle"<<std::endl;
                particleSystems[i]->update(timeClock.getElapsedTime());
                timeClock.restart();
            }
            //World::changeVisibleEntity(nullptr, nullptr);
        }
    }
}

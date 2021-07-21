#ifndef ODFAEG_RESOURCE_CACHE_HPP
#define ODFAEG_RESOURCE_CACHE_HPP
#include "resourceManager.h"
#include "export.hpp"
#include <string>
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
namespace odfaeg {
    namespace core {
        /**
        * \file resourceCache.h
        * \class ResourceCache
        * \brief manage a cache which holds resource managers of every type.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        template <typename I=std::string>
        class ODFAEG_CORE_API ResourceCache {
            public :
            /** \fn addResourceManager (ResourceManager<R, I>& rmi, std::string name)
            *   \brief add a derived resource manager to the cache.
            *   \param R resource type of the derived resource manager.
            *   \param IR alias type of the derived resource manager.
            *   \param B resource type of the base resource manager.
            *   \param IB : alias type of the base resource manager.
            *   \param ResourceManager<R, IR>& the resource manager to add.
            *   \param I name : an unique alias which is associate to the resource manager.
            */
            template <typename R, typename IR, typename B, typename IB>
            void addResourceManager(ResourceManager<R, IR>& derivedRM, const I& name) {
                std::lock_guard<std::recursive_mutex> locker(rec_mutex);
                typename std::map<I, std::unique_ptr<ResourceManagerBase>>::iterator it = resourceManagers.find(name);
                if (it != resourceManagers.end())
                    throw Erreur(1, "Identifiant already used!", 5);
                ResourceManager<B, IB> baseRM;
                std::vector<std::string> resLocalisations = derivedRM.getPaths();
                for (unsigned int i = 0; i < resLocalisations.size(); i++) {
                    R* res = const_cast<R*> (derivedRM.getResourceByPath(resLocalisations[i]));
                    typename std::vector<IR> alias = derivedRM.getAliasByResource(res);
                    if (alias.size() == 0) {
                        baseRM->make_resource(res);
                    } else {
                        for (unsigned int i = 0; i < alias.size(); i++) {
                            baseRM->make_resource(res, alias[i]);
                        }
                    }
                }
                std::unique_ptr<ResourceManagerBase> rmi = baseRM.clone();
                resourceManagers.insert(std::make_pair(name, std::move(rmi)));
            }
            /** \fn addResourceManager (ResourceManager<R, I>& rmi, std::string name)
            *   \brief add a resource manager to the class.
            *   \param R resource type of the resource manager.
            *   \param IR alias type of the resource manager.
            */
            template <typename R, typename IR>
            void addResourceManager (ResourceManager<R, IR>& baseRM, const I& name) {
                std::lock_guard<std::recursive_mutex> locker(rec_mutex);
                typename std::map<I, std::unique_ptr<ResourceManagerBase>>::iterator it = resourceManagers.find(name);
                if (it != resourceManagers.end())
                    throw Erreur(1, "Identifiant already used!", 5);
                std::unique_ptr<ResourceManagerBase> rmi = baseRM.clone();
                resourceManagers.insert(std::make_pair(name, std::move(rmi)));
            }
            /** \fn resourceManager (std::string name)
            *   \brief get a resource manager of the cache.
            *   \return the resource manager related to the given alias,
            *   throw an error if there's no resource manager associated to this alias.
            */
            template <typename R, typename IR>
            ResourceManager<R, IR>& resourceManager(const I& name) {
                std::lock_guard<std::recursive_mutex> locker(rec_mutex);
                typename std::map<I, std::unique_ptr<ResourceManagerBase>>::iterator it = resourceManagers.find(name);
                if (it == resourceManagers.end())
                    throw Erreur (12, "Resource manager not found!", 0);
                using DynamicType = ResourceManager<R, IR>*;
                return *static_cast<DynamicType> (it->second.get());
            }
            private :
            std::map<I, std::unique_ptr<ResourceManagerBase>> resourceManagers; /**> holds the resources managers and the ids.*/

        };
    }
}
#endif // RESOURCE_CACHE

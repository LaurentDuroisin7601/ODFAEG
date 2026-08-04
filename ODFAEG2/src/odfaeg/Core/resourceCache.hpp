#ifndef ODFAEG_RESOURCECACHE_HPP
#define ODFAEG_RESOURCECACHE_HPP
#include <string>
#include <memory>
#include <map>
#include <mutex>
#include <stdexcept>
#include <odfaeg/config.hpp>
#include "resourceManager.hpp"
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
        template <typename I = std::string>
        class ResourceCache {
        public:            
            /** \fn addResourceManager (ResourceManager<R, I>& rmi, std::string name)
            *   \brief add a resource manager to the class.
            *   \param R resource type of the resource manager.
            *   \param IR alias type of the resource manager.
            */
            template <typename R, typename IR>
            void addResourceManager(ResourceManager<R, IR>& baseRM, const I& name);
            /** \fn resourceManager (std::string name)
            *   \brief get a resource manager of the cache.
            *   \return the resource manager related to the given alias,
            *   throw an error if there's no resource manager associated to this alias.
            */
            template <typename R, typename IR>
            ResourceManager<R, IR>& resourceManager(const I& name);
        private:
            std::map<I, std::shared_ptr<ResourceManagerBase>> resourceManagers; /**> holds the resources managers and the ids.*/

        };
    }
}
#include "resourceCache.inl"
#endif

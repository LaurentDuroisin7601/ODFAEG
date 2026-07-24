namespace odfaeg {
    namespace core {              
        /** \fn addResourceManager (ResourceManager<R, I>& rmi, std::string name)
        *   \brief add a resource manager to the class.
        *   \param R resource type of the resource manager.
        *   \param IR alias type of the resource manager.
        */
        template <typename I>
        template <typename R, typename IR>
        void ResourceCache<I>::addResourceManager(ResourceManager<R, IR>& baseRM, const I& name) {
            std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
            typename std::map<I, std::shared_ptr<ResourceManagerBase>>::iterator it = resourceManagers.find(name);
            if (it != resourceManagers.end())
                throw std::runtime_error("Identifiant already used!");
            std::shared_ptr<ResourceManagerBase> rmi = baseRM.clone();
            resourceManagers.insert(std::make_pair(name, rmi));
        }
        /** \fn resourceManager (std::string name)
        *   \brief get a resource manager of the cache.
        *   \return the resource manager related to the given alias,
        *   throw an error if there's no resource manager associated to this alias.
        */
        template <typename I>
        template <typename R, typename IR>
        ResourceManager<R, IR>& ResourceCache<I>::resourceManager(const I& name) {
            std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
            typename std::map<I, std::unique_ptr<ResourceManagerBase>>::iterator it = resourceManagers.find(name);
            if (it == resourceManagers.end())
                throw std::runtime_error("Resource manager not found!");
            using DynamicType = ResourceManager<R, IR>*;
            return *static_cast<DynamicType> (it->second.get());
        }
    }
}
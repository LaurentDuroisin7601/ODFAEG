namespace odfaeg {
    namespace core {
               
            template <typename R, typename I>
            template <typename... RA, size_t... Inds>
            std::shared_ptr<R> ResourceManager<R, I>::createResourceFromTuple(std::tuple<std::reference_wrapper<RA>...> rArgs, std::index_sequence<Inds...> seq) {
                std::shared_ptr<R> resource = std::make_shared<R>(std::get<Inds>(rArgs).get()...);
                return resource;
            }         
            template <typename R, typename I>   
            void ResourceManager<R, I>::make_resource(R* resource) {
                std::string path = conversionLongString(reinterpret_cast<unsigned long long int>(resource));
                std::shared_ptr<R> res;
                res.reset(resource);
                insertResource(path, resource);
            }   
            template <typename R, typename I>         
            void ResourceManager<R, I>::make_resource(R* resource, const I& alias) {
                //std::cout<<"resource : "<<resource<<std::endl;

                std::string path = conversionLongString(reinterpret_cast<unsigned long long int>(resource));
                //std::cout<<"path : "<<path<<std::endl;
                std::shared_ptr<R> res;
                res.reset(resource);
                insertResource(path, res, alias);
            }  
            template <typename R, typename I>           
            template <typename... A>
            void ResourceManager<R, I>::fromFile(const std::string& path, A... args)
            {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                std::shared_ptr<R> resource = std::make_shared<R>();
                insertResource(path, resource);
            }     
            template <typename R, typename I>       
            template <typename... A, typename... RA>
            void ResourceManager<R, I>::fromFileWithAlias(const std::string& path, const I& alias, A... args, std::tuple<std::reference_wrapper<RA>...> rArgs)
            {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                // Create and load resource
                std::shared_ptr<R> resource = createResourceFromTuple(rArgs, std::make_index_sequence<sizeof...(RA)>());
                //std::cout<<"resource : "<<resource<<std::endl;

                if (!resource->loadFromFile(path, args...)) {
                    throw std::runtime_error("ResourceManager::load - Failed to load " + path);
                }
                // If loading successful, insert resource to map
                insertResource(path, resource, alias);
            }     
            template <typename R, typename I>      
            template <typename... A, typename... RA>
            void ResourceManager<R, I>::fromMemory(void* localisation, std::size_t size, A... args, std::tuple<std::reference_wrapper<RA>...> rArgs)
            {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                std::shared_ptr<R> resource = createResourceFromTuple(rArgs, std::make_index_sequence<sizeof...(RA)>());
                if (!resource->loadFromMemory(localisation, size, args...)) {
                    throw std::runtime_error("ResourceManager::load - Failed to load ");
                }
                insertResource(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)), resource);
            }     
            template <typename R, typename I>       
            template <typename... A, typename... RA>
            void ResourceManager<R, I>::fromMemoryWithAlias(void* localisation, std::size_t size, const I& alias, A... args, std::tuple<std::reference_wrapper<RA>...> rArgs)
            {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                std::shared_ptr<R> resource = createResourceFromTuple(rArgs, std::make_index_sequence<sizeof...(RA)>());

                if (!resource->loadFromMemory(localisation, size, args...)) {
                    // If loading successful, insert resource to map
                    insertResource(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)), resource, alias);
                }
            }
            template <typename R, typename I>
            template <typename... A>
            void ResourceManager<R, I>::fromFile(std::function<bool(R*, A...)> func, const std::string& path, A... args) {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                std::shared_ptr<R> resource = std::make_shared<R>();
                if (!func(resource, path, args...)) {
                    throw std::runtime_error("ResourceManager::load - Failed to load " + path);
                    delete resource;
                }
                insertResource(path, resource);
            }       
            template <typename R, typename I>     
            template <typename... A>
            void ResourceManager<R, I>::fromFileWithAlias(std::function<bool(R*, A...)> func, const std::string& path, const I& alias, A... args)
            {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                // Create and load resource
                std::shared_ptr<R> resource = std::make_shared<R>();
                if (!func(resource, path, args...)) {
                    throw std::runtime_error("ResourceManager::load - Failed to load ");
                }
                // If loading successful, insert resource to map
                insertResource(path, resource, alias);
            }    
            template <typename R, typename I>        
            template <typename... A, typename L>
            void ResourceManager<R, I>::fromMemory(std::function<bool(R*, A...)> func, L* localisation, A... args) {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                std::shared_ptr<R> resource = std::make_unique<R>();
                if (!func(resource, localisation, args...)) {
                    throw std::runtime_error("ResourceManager::load - Failed to load ");
                }
                insertResource(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)), resource);
            }       
            template <typename R, typename I>     
            template <typename... A, typename L>
            void ResourceManager<R, I>::fromMemoryWithAlias(std::function<bool(R*, A...)> func, L* localisation, const I& alias, A... args)
            {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                // Create and load resource
                std::shared_ptr<R> resource = std::make_unique<R>();
                typename std::unordered_map<I, unsigned int>::iterator it = mAliasMap.find(alias);
                if (!func(resource, localisation, args...)) {
                    throw std::runtime_error("ResourceManager::load - Failed to load ");
                }
                // If loading successful, insert resource to map
                insertResource(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)), resource, alias);
            }
            template <typename R, typename I>
            const unsigned int& ResourceManager<R, I>::getIdByAlias(const I& alias) const {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                typename std::unordered_map<I, unsigned int>::const_iterator it = mAliasMap.find(alias);
                if (it != mAliasMap.end()) {
                     return it->second;
                }
                throw std::runtime_error("Alias not found!");
            }    
            template <typename R, typename I>        
            const unsigned int& ResourceManager<R, I>::getIdByResource(const R* resource) const {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                auto it = mResourceMap.begin();
                for (unsigned int i = 0; i < mResourceMap.size(); i++) {
                    if (it->second.getResource() == resource) {
                        return i;
                    }
                    mResourceMap.advance(it, i+1);
                }
                throw std::runtime_error("Id not found!");
            }       
            template <typename R, typename I>     
            const unsigned int& ResourceManager<R, I>::getIdByPath(const std::string& path) const {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                auto it = mResourceMap.find(path);
                if (it != mResourceMap.end()) {
                    unsigned int id = getIdByResource(it->second.getResource());
                    return getResourceById(id);
                }
                throw std::runtime_error("Path not found!");
            }    
            template <typename R, typename I>        
            R* ResourceManager<R, I>::getResourceById(const unsigned int id) const
            {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                if (id >= mResourceMap.size())
                    throw std::runtime_error("Id not found!");
                auto it = mResourceMap.begin();
                // 2. Avancer l'itérateur de i positions
                std::advance(it, id);
                if (it != mResourceMap.end())
                    return it->second.getResource();
                throw std::runtime_error("Resource not found!");
            }  
            template <typename R, typename I>          
            R* ResourceManager<R, I>::getResourceByAlias(const I& alias) const
            {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                typename std::unordered_map<I, std::string>::const_iterator it = mAliasMap.find(alias);
                if (it != mAliasMap.end()) {
                    auto it2 = mResourceMap.find(it->second);
                    if (it2 != mResourceMap.end()) {
                        return it2->second.getResource();
                    }
                    throw std::runtime_error("Resource not found!");
                }
                throw std::runtime_error("Alias not found!");
            }     
            template <typename R, typename I>       
            R* ResourceManager<R, I>::getResourceByPath(const std::string& path) const
            {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                auto it = mResourceMap.find(path);
                if (it != mResourceMap.end()) {
                    return it->getResource();
                }
                throw std::runtime_error("Path not found!");
            }    
            template <typename R, typename I>        
            std::string ResourceManager<R, I>::getPathById(const unsigned int id) const {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                typename std::vector<Resource>::iterator it;
                R* resource = getResourceById(id);
                if (resource != nullptr)
                    return resource->getPath();
                throw std::runtime_error("Id not found!");
            }    
            template <typename R, typename I>        
            std::string ResourceManager<R, I>::getPathByAlias(const I& alias) const {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                typename std::unordered_map<I, std::string>::iterator it = mAliasMap.find(alias);
                if (it != mAliasMap.end()) {
                    return it->second;
                }
            }  
            template <typename R, typename I>          
            std::string ResourceManager<R, I>::getPathByResource(const R* resource) const {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                for (const std::pair<std::string, Resource>& pair : mResourceMap) {
                    if (pair.second.getResource() == resource) {
                        return pair.second.getPath();
                    }
                }
                throw std::runtime_error("Resource not found!");
            }     
            template <typename R, typename I>
            std::vector<I> ResourceManager<R, I>::getAliasByResource(R* resource) {
                std::vector<I> aliases;
                std::string path = getPathByResource(resource);
                typename std::unordered_map<I, std::string> alias;
                for (const std::pair<I, std::string>& pair : mAliasMap) {
                    if (pair.second == path) {
                        alias.push_back(pair.first);
                    }
                }
                return alias;
            }      
            template <typename R, typename I>      
            std::vector<std::string> ResourceManager<R, I>::getPaths() {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                std::vector<std::string> paths;
                for (auto& pair : mResourceMap) {
                    paths.push_back(pair.first);
                }
                return paths;
            }
            template <typename R, typename I>
            std::vector<I> ResourceManager<R, I>::getAliases() {
                std::vector<I> aliases;
                for (auto& pair : mAliasMap) {
                    aliases.push_back(pair.first);
                }
                return aliases;
            }
            template <typename R, typename I>
            void ResourceManager<R, I>::insertResource(const std::string& path, std::shared_ptr<R>& resource, const I& alias)
            {
                //std::cout<<"insert resource : "<<alias<<std::endl;
                //std::cout<<"resource size : "<<mResourceMap.size()<<std::endl;

                // Insert and check success
                auto it = mResourceMap.find(path);
                mResourceMap.insert(std::make_pair(path, Resource(path, resource, nbResources)));
                mAliasMap.insert(std::make_pair(alias, path));
                if (it == mResourceMap.end()) {
                    nbResources++;
                }
            }
            template <typename R, typename I>
            bool ResourceManager<R, I>::exist(std::string path) {
                auto it = mResourceMap.find(path);
                return (it != mResourceMap.end());
            }
            template <typename R, typename I>
            bool ResourceManager<R, I>::existFromAlias(I alias) {
                typename std::unordered_map<I, std::string>::iterator it = mAliasMap.find(alias);
                if (it != mAliasMap.end()) {
                    return exist(it->second);
                }
                return false;
            }
            template <typename R, typename I>
            void ResourceManager<R, I>::insertResource(const std::string path, std::shared_ptr<R> resource)
            {
                //std::cout<<"insert resource : "<<path<<std::endl;
                typename std::unordered_map<std::string, Resource>::iterator it = mResourceMap.find(path);
                mResourceMap.insert(std::make_pair(path, Resource(path, resource, nbResources)));
                if (it == mResourceMap.end()) {
                    nbResources++;
                }
            }           
            template <typename R, typename I> 
            void ResourceManager<R, I>::deleteResourceById(const unsigned int id) {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                R* resource = const_cast<R*>(getResourceById(id));
                deleteResource(resource);
            } 
            template <typename R, typename I>           
            void ResourceManager<R, I>::deleteResourceByPath(const std::string& path) {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                R* resource = const_cast<R*>(getResourceByPath(path));
                deleteResource(resource);
            }
            template <typename R, typename I>            
            void ResourceManager<R, I>::deleteResourceByAlias(const I& alias) {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                R* resource = const_cast<R*> (getResourceByAlias(alias));
                deleteResource(resource);
            } 
            template <typename R, typename I>           
            void ResourceManager<R, I>::deleteResource(R* resource) {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                I alias = getAliasByResource(resource);
                typename std::unordered_map<I, std::string>::iterator it = mAliasMap.find(alias);
                if (it != mAliasMap.end()) {
                    mAliasMap.erase(it);
                    typename std::unordered_map<I, std::string>::iterator it2 = mResourceMap.find(it->second);
                    if (it2 != mResourceMap.end()) {
                        mResourceMap.erase(it2);
                    }
                } else {
                    unsigned int id = getIdByResource(resource);
                    mResourceMap.advance(it, id);
                    if (it != mResourceMap.edn()) {
                        mResourceMap.erase(it);
                    }
                }
                delete resource;
                resource = nullptr;
            }  
            template <typename R, typename I>          
            void ResourceManager<R, I>::deleteAll() {
                mResourceMap.clear();
                mAliasMap.clear();
            }  
            template <typename R, typename I>          
            std::shared_ptr<ResourceManagerBase> ResourceManager<R, I>::clone() {
                return std::make_shared<ResourceManager<R, I>>(*this);
            }
            template <typename R, typename I>
            ResourceManager<R, I>::ResourceManager(const ResourceManager& rm) {
                mResourceMap = rm.mResourceMap;
                mAliasMap = rm.mAliasMap;
                nbResources = rm.nbResources;
            }
            /** \fn ResourceManager& operator=(const ResourceManager& rm)
            *   \brief operation affector.
            *   \param const ResourceManager& the resource manager to copy.
            *   \return the resource manager to affect.
            */
            template <typename R, typename I>
            ResourceManager<R, I>& ResourceManager<R, I>::operator=(const ResourceManager& rm) {
                mResourceMap = rm.mResourceMap;
                mAliasMap = rm.mAliasMap;
                nbResources = rm.nbResources;
                return *this;
            }
        }
    }
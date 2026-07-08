module;
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <odfaeg/config.hpp>
#include <algorithm>
export module odfaeg.core.resourceManager;
import odfaeg.core.utilities;
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
export namespace odfaeg
{
    namespace core {
        /**
        * \file ResourceManager.h
        * \class ResourceManagerBase
        * \brief Base class used for the type erasure.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ResourceManagerBase {
        public:
            virtual ~ResourceManagerBase() {}
        };
        /**
        * \file ResourceManager.h
        * \class ResourceManager
        * \brief Class used to store every resources of a particular type, and, associate them with an alias of any type.
        * By default the type of the alias is an std::string.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        template<typename R, typename I = std::string>
        class ResourceManager : public ResourceManagerBase
        {
        public:
            ResourceManager() : nbResources(0) {
                //std::cout<<"create resource maanger"<<std::endl;
            }
            template <typename... RA, size_t... Inds>
            std::shared_ptr<R> createResourceFromTuple(std::tuple<std::reference_wrapper<RA>...> rArgs, std::index_sequence<Inds...> seq) {
                std::shared_ptr<R> resource = std::make_shared<R>(std::get<Inds>(rArgs).get()...);
                return resource;
            }            
            void make_resource(R* resource) {
                std::string path = conversionLongString(reinterpret_cast<unsigned long long int>(resource));
                std::shared_ptr<R> res;
                res.reset(resource);
                insertResource(path, resource);
            }            
            void make_resource(R* resource, const I& alias) {
                //std::cout<<"resource : "<<resource<<std::endl;

                std::string path = conversionLongString(reinterpret_cast<unsigned long long int>(resource));
                //std::cout<<"path : "<<path<<std::endl;
                std::shared_ptr<R> res;
                res.reset(resource);
                insertResource(path, res, alias);
            }            
            template <typename... A>
            void fromFile(const std::string& path, A... args)
            {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                std::shared_ptr<R> resource = std::make_shared<R>();
                insertResource(path, resource);
            }            
            template <typename... A, typename... RA>
            void fromFileWithAlias(const std::string& path, const I& alias, A... args, std::tuple<std::reference_wrapper<RA>...> rArgs)
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
            template <typename... A, typename... RA>
            void fromMemory(void* localisation, std::size_t size, A... args, std::tuple<std::reference_wrapper<RA>...> rArgs)
            {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                std::shared_ptr<R> resource = createResourceFromTuple(rArgs, std::make_index_sequence<sizeof...(RA)>());
                if (!resource->loadFromMemory(localisation, size, args...)) {
                    throw std::runtime_error("ResourceManager::load - Failed to load ");
                }
                insertResource(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)), resource);
            }            
            template <typename... A, typename... RA>
            void fromMemoryWithAlias(void* localisation, std::size_t size, const I& alias, A... args, std::tuple<std::reference_wrapper<RA>...> rArgs)
            {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                std::shared_ptr<R> resource = createResourceFromTuple(rArgs, std::make_index_sequence<sizeof...(RA)>());

                if (!resource->loadFromMemory(localisation, size, args...)) {
                    // If loading successful, insert resource to map
                    insertResource(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)), resource, alias);
                }
            }
            
            template <typename... A>
            void fromFile(std::function<bool(R*, A...)> func, const std::string& path, A... args) {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                std::shared_ptr<R> resource = std::make_shared<R>();
                if (!func(resource, path, args...)) {
                    throw std::runtime_error("ResourceManager::load - Failed to load " + path);
                    delete resource;
                }
                insertResource(path, resource);
            }            
            template <typename... A>
            void fromFileWithAlias(std::function<bool(R*, A...)> func, const std::string& path, const I& alias, A... args)
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
            template <typename... A, typename L>
            void fromMemory(std::function<bool(R*, A...)> func, L* localisation, A... args) {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                std::shared_ptr<R> resource = std::make_unique<R>();
                if (!func(resource, localisation, args...)) {
                    throw std::runtime_error("ResourceManager::load - Failed to load ");
                }
                insertResource(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)), resource);
            }            
            template <typename... A, typename L>
            void fromMemoryWithAlias(std::function<bool(R*, A...)> func, L* localisation, const I& alias, A... args)
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
            const unsigned int& getIdByAlias(const I& alias) const {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                typename std::unordered_map<I, unsigned int>::const_iterator it = mAliasMap.find(alias);
                if (it != mAliasMap.end()) {
                     return it->second;
                }
                throw std::runtime_error("Alias not found!");
            }            
            const unsigned int& getIdByResource(const R* resource) const {
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
            const unsigned int& getIdByPath(const std::string& path) const {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                auto it = mResourceMap.find(path);
                if (it != mResourceMap.end()) {
                    unsigned int id = getIdByResource(it->second.getResource());
                    return getResourceById(id);
                }
                throw std::runtime_error("Path not found!");
            }            
            R* getResourceById(const unsigned int id) const
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
            R* getResourceByAlias(const I& alias) const
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
            R* getResourceByPath(const std::string& path) const
            {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                auto it = mResourceMap.find(path);
                if (it != mResourceMap.end()) {
                    return it->getResource();
                }
                throw std::runtime_error("Path not found!");
            }            
            std::string getPathById(const unsigned int id) const {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                typename std::vector<Resource>::iterator it;
                R* resource = getResourceById(id);
                if (resource != nullptr)
                    return resource->getPath();
                throw std::runtime_error("Id not found!");
            }            
            std::string getPathByAlias(const I& alias) const {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                typename std::unordered_map<I, std::string>::iterator it = mAliasMap.find(alias);
                if (it != mAliasMap.end()) {
                    return it->second;
                }
            }            
            std::string getPathByResource(const R* resource) const {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                for (const std::pair<std::string, Resource>& pair : mResourceMap) {
                    if (pair.second.getResource() == resource) {
                        return pair.second.getPath();
                    }
                }
                throw std::runtime_error("Resource not found!");
            }           
            std::vector<I> getAliasByResource(R* resource) {
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
            std::vector<std::string> getPaths() {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                std::vector<std::string> paths;
                for (auto& pair : mResourceMap) {
                    paths.push_back(pair.first);
                }
                return paths;
            }
            std::vector<I> getAliases() {
                std::vector<I> aliases;
                for (auto& pair : mAliasMap) {
                    aliases.push_back(pair.first);
                }
                return aliases;
            }
            void insertResource(const std::string& path, std::shared_ptr<R>& resource, const I& alias)
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
            bool exist(std::string path) {
                auto it = mResourceMap.find(path);
                return (it != mResourceMap.end());
            }
            bool existFromAlias(I alias) {
                typename std::unordered_map<I, std::string>::iterator it = mAliasMap.find(alias);
                if (it != mAliasMap.end()) {
                    return exist(it->second);
                }
                return false;
            }
            void insertResource(const std::string path, std::shared_ptr<R> resource)
            {
                //std::cout<<"insert resource : "<<path<<std::endl;
                typename std::unordered_map<std::string, Resource>::iterator it = mResourceMap.find(path);
                mResourceMap.insert(std::make_pair(path, Resource(path, resource, nbResources)));
                if (it == mResourceMap.end()) {
                    nbResources++;
                }
            }            
            void deleteResourceById(const unsigned int id) {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                R* resource = const_cast<R*>(getResourceById(id));
                deleteResource(resource);
            }            
            void deleteResourceByPath(const std::string& path) {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                R* resource = const_cast<R*>(getResourceByPath(path));
                deleteResource(resource);
            }            
            void deleteResourceByAlias(const I& alias) {
                //std::unique_lock<std::recursive_mutex> locker(getGlobalMutex());
                R* resource = const_cast<R*> (getResourceByAlias(alias));
                deleteResource(resource);
            }            
            void deleteResource(R* resource) {
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
            void deleteAll() {
                mResourceMap.clear();
                mAliasMap.clear();
            }            
            std::shared_ptr<ResourceManagerBase> clone() {
                return std::make_shared<ResourceManager<R, I>>(*this);
            }
            ResourceManager(const ResourceManager& rm) {
                mResourceMap = rm.mResourceMap;
                mAliasMap = rm.mAliasMap;
                nbResources = rm.nbResources;
            }
            /** \fn ResourceManager& operator=(const ResourceManager& rm)
            *   \brief operation affector.
            *   \param const ResourceManager& the resource manager to copy.
            *   \return the resource manager to affect.
            */
            ResourceManager& operator=(const ResourceManager& rm) {
                mResourceMap = rm.mResourceMap;
                mAliasMap = rm.mAliasMap;
                nbResources = rm.nbResources;
                return *this;
            }
        private:
            /*ResourceManager(const ResourceManager& rm) = delete;
            ResourceManager& operator=(const ResourceManager& rm) = delete;*/
            /**
            * \file ResourceManager.h
            * \class Resource
            * \brief SubClass which'll old the pointer to the resource.
            * \author Duroisin.L
            * \version 1.0
            * \date 1/02/2014
            */

            struct Resource {
            public:

                /**
                *\fn constructor.
                *\param const std::string path : the path of the resource.
                *\param R* resource : the pointer to the resource.
                */
                Resource(const std::string path, std::shared_ptr<R> resource, int nbResources) :
                    path(path),
                    resource(resource),
                    id(nbResources)
                {
                    //std::cout<<"resource path : "<<path<<std::endl;
                }
                /**
                *\fn R& getResource();
                *\return R& a reference to the resource.
                */
                R* getResource() const {
                    return resource.get();
                }
                /**
                *\fn std::string getPath();
                *\return std::string the path to the resource.
                */
                std::string getPath() const {
                    return path;
                }
                /**
                *\fn unsigned int& getId();
                *\return the resource's id.
                */
                const unsigned int& getId() const {
                    return id;
                }
            private:
                std::shared_ptr<R> resource; /**> holds a pointer to the resource.*/
                std::string path; /**> holds the path of the resource.*/
                unsigned int id; /**> holds the id of the resource.*/
            };
            unsigned int nbResources; /**> count the number of the resources which are loaded.*/
            std::unordered_map<std::string, Resource>    mResourceMap; /**> holds the resources and their id.*/
            std::unordered_map<I, std::string>  mAliasMap; /**> holds the resource's id and their associated alias.*/
        };
        ///////////////////////////////////////////////////////////////////////////////
        // IMPLEMENTATION                                                            //
        ///////////////////////////////////////////////////////////////////////////////
       
        template <typename B, typename I = std::string>
        class BaseResourceManager : public ResourceManager<B, I> {
        public:
            std::shared_ptr<ResourceManagerBase> clone() {
                return std::make_shared<BaseResourceManager<B, I>>(*this);
            }
            virtual ~BaseResourceManager() {}
        };
    }
}
////////////////
/*export namespace odfaeg {
    namespace core {
        template <typename I = std::string> using TextureManager = ResourceManager<odfaeg::graphic::Texture, I>;
        template <typename I = std::string> using ShaderManager = ResourceManager<odfaeg::graphic::Shader, I>;
        template <typename I = std::string> using FontManager = ResourceManager<odfaeg::graphic::Font, I>;
        template <typename I = std::string> using SoundBufferManager = ResourceManager<odfaeg::audio::SoundBuffer, I>;
    }
}*/



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
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
namespace odfaeg
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
            std::shared_ptr<R> createResourceFromTuple(std::tuple<std::reference_wrapper<RA>...> rArgs, std::index_sequence<Inds...> seq);
            void make_resource(R* resource);
            void make_resource(R* resource, const I& alias);
            template <typename... A>
            void fromFile(const std::string& path, A... args);
            template <typename... A, typename... RA>
            void fromFileWithAlias(const std::string& path, const I& alias, A... args, std::tuple<std::reference_wrapper<RA>...> rArgs);
            template <typename... A, typename... RA>
            void fromMemory(void* localisation, std::size_t size, A... args, std::tuple<std::reference_wrapper<RA>...> rArgs);           
            template <typename... A, typename... RA>
            void fromMemoryWithAlias(void* localisation, std::size_t size, const I& alias, A... args, std::tuple<std::reference_wrapper<RA>...> rArgs);
            
            template <typename... A>
            void fromFile(std::function<bool(R*, A...)> func, const std::string& path, A... args);           
            template <typename... A>
            void fromFileWithAlias(std::function<bool(R*, A...)> func, const std::string& path, const I& alias, A... args);           
            template <typename... A, typename L>
            void fromMemory(std::function<bool(R*, A...)> func, L* localisation, A... args);           
            template <typename... A, typename L>
            void fromMemoryWithAlias(std::function<bool(R*, A...)> func, L* localisation, const I& alias, A... args);
            const unsigned int& getIdByAlias(const I& alias) const;
            const unsigned int& getIdByResource(const R* resource) const;
            const unsigned int& getIdByPath(const std::string& path) const;
            R* getResourceById(const unsigned int id) const;
            R* getResourceByAlias(const I& alias) const;
            R* getResourceByPath(const std::string& path) const;
            std::string getPathById(const unsigned int id) const;
            std::string getPathByAlias(const I& alias) const;
            std::string getPathByResource(const R* resource) const;
            std::vector<I> getAliasByResource(R* resource);
            std::vector<std::string> getPaths();
            std::vector<I> getAliases();
            void insertResource(const std::string& path, std::shared_ptr<R>& resource, const I& alias);
            bool exist(std::string path);
            bool existFromAlias(I alias);
            void insertResource(const std::string path, std::shared_ptr<R> resource);
            void deleteResourceById(const unsigned int id);
            void deleteResourceByPath(const std::string& path);
            void deleteResourceByAlias(const I& alias);
            void deleteResource(R* resource);
            void deleteAll();
            std::shared_ptr<ResourceManagerBase> clone();
            ResourceManager(const ResourceManager& rm);
            /** \fn ResourceManager& operator=(const ResourceManager& rm)
            *   \brief operation affector.
            *   \param const ResourceManager& the resource manager to copy.
            *   \return the resource manager to affect.
            */
            ResourceManager& operator=(const ResourceManager& rm);
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
#include "resourceManager.inl"
////////////////
/*export namespace odfaeg {
    namespace core {
        template <typename I = std::string> using TextureManager = ResourceManager<odfaeg::graphic::Texture, I>;
        template <typename I = std::string> using ShaderManager = ResourceManager<odfaeg::graphic::Shader, I>;
        template <typename I = std::string> using FontManager = ResourceManager<odfaeg::graphic::Font, I>;
        template <typename I = std::string> using SoundBufferManager = ResourceManager<odfaeg::audio::SoundBuffer, I>;
    }
}*/



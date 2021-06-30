#ifndef ODFAEG_RESOURCEMANAGER_HPP
#define ODFAEG_RESOURCEMANAGER_HPP

#include <map>
#include <string>
#include <stdexcept>
#include <iostream>
#include "erreur.h"
#include <vector>
#include <functional>
#include "export.hpp"
#include <memory>
#include "singleton.h"
#include "utilities.h"
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
                /**
                *  \fn ResourceManager()
                *  \brief constructor, initialize variables.
                */
                ResourceManager() {
                    nbResources = 0;
                }
                /**
                *  \fn void make_resource(R* resource)
                *  \brief add the resource to the resource manager.
                *  \param R* the resource to add.
                */
                void make_resource(R* resource);
                /**
                *  \fn void make_resource(R* resource)
                *  \brief add the resource to the resource manager.
                *  \param R* the resource to add.
                *  \param I the alias pointing to the resource.
                */
                void make_resource(R* resource, const I& alias);
                /** \fn  fromFile(const std::string& path)
                *   \brief load a resource from a file.
                *   \param the location of the file. (Relative path)
                */
                template <typename... A>
                void                      fromFile(const std::string& path, A... args);
                /** \fn fromFile(const std::string& path, const I& alias)
                *   \brief load a resource from a file.
                *   \param the location of the file. (Relative path)
                *   \param const I& alias : an alias associated to the resource.
                */
                template <typename... A>
                void                      fromFileWithAlias(const std::string& path, const I& alias, A... args);
                 /** \fn  fromFile(const std::string& path)
                *   \brief load a resource from a file.
                *   \param the location of the file. (Relative path)
                */
                template <typename... A, typename L>
                void                      fromMemoryWithAlias(L* localisation, const I& alias, A... args);
                /** \fn  fromFile(const std::string& path)
                *   \brief load a resource from a file.
                *   \param the location of the file. (Relative path)
                */
                template <typename... A, typename L>
                void                      fromMemory(L* localisation, A... args);
                /** \fn void load(std::function<bool(R*, A...)> func, A... args);
                *   \brief load a resource with the given function.
                *   \param std::function<bool(R*, A...)> func : the function which load the resource.
                *   \param A... args : the arguments of the function.
                */
                template <typename ...A>
                void                      fromFile(std::function<bool(R*, A...)> func, const std::string& path, A... args);
                 /** \fn void load(std::function<bool(R*, A...)> func, A... args, const I& alias);
                *   \brief load a resource with the given function.
                *   \param std::function<bool(R*, A...)> func : the function which load the resource.
                *   \param A... args : the arguments of the function.
                *   \param const I& alias : an alias associated to the resource.
                */
                template <typename ...A>
                void                      fromFileWithAlias(std::function<bool(R*, A...)> func, const std::string& path, const I& alias, A... args);
                /** \fn void load(std::function<bool(R*, A...)> func, A... args);
                *   \brief load a resource with the given function.
                *   \param std::function<bool(R*, A...)> func : the function which load the resource.
                *   \param A... args : the arguments of the function.
                */
                template <typename ...A, typename L>
                void                      fromMemory(std::function<bool(R*, A...)> func, L* localisation, A... args);
                 /** \fn void load(std::function<bool(R*, A...)> func, A... args, const I& alias);
                *   \brief load a resource with the given function.
                *   \param std::function<bool(R*, A...)> func : the function which load the resource.
                *   \param A... args : the arguments of the function.
                *   \param const I& alias : an alias associated to the resource.
                */
                template <typename ...A, typename L>
                void                      fromMemoryWithAlias(std::function<bool(R*, A...)> func, L* localisation, const I& alias, A... args);
                /** \fn R* getResourceByPath (const std::string path) const;
                *   \brief get the resource by path.
                *   \param std::string& path : the path of the resource to get.
                *   \return R* a pointer to the resource.
                */
                bool                      exists(const std::string& path) const;
                bool                      exists(const unsigned int id) const;
                const R*                  getResourceByPath(const std::string& path) const;
                /** \fn R* getResourceById (const int& id) const;
                *   \brief get the resource by its id.
                *   (The first resource loaded have the id 0, the second, the id 1, etc...
                *   if the first resource is deleted, the second resource takes the id 0, the thirst resource takes the id 1, etc...)
                *   \param const int &id : the id of the resource to get.
                *   \return R* a pointer to the resource.
                */
                const R*                  getResourceById(const unsigned int id) const;
                /** \fn R* getResourceByAlias (const I& alias) const;
                *   \brief get the resource by its alias.
                *   \param const I& alias : the alias of the resource.
                *   \return R* a pointer to the resource.
                */
                const R*                  getResourceByAlias(const I &alias) const;
                 /** \fn std::string getPathById(const int& id) const;
                *   \brief get the path of the resource by it's id.
                *   \param const I& alias : the alias of the resource.
                *   \return std::string the path of the resource.
                */
                std::string               getPathById (const unsigned int id) const;
                /** \fn std::string getPathByAlias(const I& alias) const;
                *   \brief get the path of the resource by it's alias.
                *   \param const I& alias : the alias of the resource.
                *   \return std::string the path of the resource.
                */
                std::string               getPathByAlias (const I &alias) const;
                /** \fn std::string getPathByResource(const R* resource) const;
                *   \brief get the path of the resource.
                *   \param const R* resource : the resource.
                *   \return std::string the path of the resource.
                */
                std::string               getPathByResource (const R* resource) const;
                /** \fn int getIdByAlias (const I& alias) const;
                *   \brief get the resource id associated to the given alias.
                *   \param const I& alias : the alias.
                *   \return int : the id of the resource.
                */
                const unsigned int&                       getIdByAlias (const I &alias) const;
                /** \fn int getIdByPath (const I& alias) const;
                *   \brief get the resource id associated to the given path.
                *   \param const std::string& path : the path.
                *   \return int : the id of the resource.
                */
                const unsigned int&                       getIdByPath (const std::string &path) const;
                /** \fn int getIdByResource (const R* resource) const;
                *   \brief get the id associated to the given resource.
                *   \param R* the resource.
                *   \return int : the id of the resource.
                */
                const unsigned int&                       getIdByResource (const R* resource) const;
                /** \fn std::vector<std::string> getPaths () const;
                *   \brief get the paths of  each resources.
                *   \return std::vector<std::string> the list of the path of each resource.
                */
                std::vector<I> getAliasByResource(R* resource);
                std::vector<std::string>  getPaths ();
                /** \fn deleteResourceByAlias(const I& alias)
                *   \brief delete a resource by its alias.
                *   \param const I& alias : the alias of the resource.
                */
                void                      deleteResourceByAlias (const I& alias);
                /** \fn deleteResourceByPath(const std::string& path)
                *   \brief delete a resource by its path.
                *   \param const std::string path : the path of the resource.
                */
                void                      deleteResourceByPath (const std::string &path);
                /** \fn deleteResourceById(const int& id)
                *   \brief delete a resource by its id.
                *   \param const int& id : the id of the resource.
                */
                void                      deleteResourceById (const unsigned int id);
                /** \fn deleteResource(R* resource)
                *   \brief delete the resource.
                *   \param R* resource : the resource.
                */
                void                      deleteResource(R* resource);
                /** \fn void deleteAll();
                *   \brief delete all resources.
                */
                void deleteAll();
                /** \fn std::unique_ptr<ResourceManagerBase> clone()
                *   \brief clone the resource manager.
                *   \return the cloned resource manager.
                */
                virtual std::unique_ptr<ResourceManagerBase> clone();
                /** \fn ResourceManager(const ResourceManager& rm)
                *   \brief copy constructor
                *   \param const ResourceManager& the resource manager to copy.
                */
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
                private :
                /**
                * \file ResourceManager.h
                * \class Resource
                * \brief SubClass which'll old the pointer to the resource.
                * \author Duroisin.L
                * \version 1.0
                * \date 1/02/2014
                */
                struct Resource {
                    public :

                    /**
                    *\fn constructor.
                    *\param const std::string path : the path of the resource.
                    *\param R* resource : the pointer to the resource.
                    */
                    Resource (const std::string path, std::unique_ptr<R>&& resource, int nbResources) :
                    path(path),
                    resource(std::move(resource)),
                    id(nbResources)
                    {

                    }
                    /**
                    *\fn copy constructor.
                    *\param const Resource: the resource to copy.
                    */
                    Resource (const Resource& other) {
                        path = other.path;
                        resource.reset(const_cast<Resource&>(other).resource.release());
                        id = other.id;
                    }
                    /**
                    *\fn Resource& operator= (const Resource& other)
                    *\param const Resource: the resource to copy.
                    *\return Resource& the resource to affect.
                    */
                    Resource& operator= (const Resource& other) {
                        path = other.path;
                        resource.reset(const_cast<Resource&>(other).resource.release());
                        id = other.id;
                        return *this;
                    }
                    /**
                    *\fn R& getResource();
                    *\return R& a reference to the resource.
                    */
                    const R* getResource() const {
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
                    std::unique_ptr<R> resource; /**> holds a pointer to the resource.*/
                    std::string path; /**> holds the path of the resource.*/
                    unsigned int id; /**> holds the id of the resource.*/
                };
                /** \fn insertResource (const std::string& path, R* resource, const I& alias)
                *   \brief insert a resource into the map.
                *   \param const std::string& path : the path of the resource.
                *   \param R* resource : a pointer to the resource.
                *   \param const I& alias : an alias associated to the resource.
                */
                void insertResource(const std::string &path, std::unique_ptr<R>&& resource, const I &alias);
                /** \fn insertResource (const std::string& path, R* resource, const I& alias)
                *   \brief insert a resource into the map.
                *   \param const std::string& path : the path of the resource.
                *   \param R* resource : a pointer to the resource.
                */
                void insertResource(const std::string &path, std::unique_ptr<R>&& resource);
                /** \fn void updateIds(const int &id)
                *   \brief updates the id of the resources. (When a resource is deleted)
                *   \param const int& id : the id of the resource.
                */

                unsigned int nbResources; /**> count the number of the resources which are loaded.*/
                std::vector<Resource>    mResourceMap; /**> holds the resources and their id.*/
                std::map<I, unsigned int>  mAliasMap; /**> holds the resource's id and their associated alias.*/
        };
        ///////////////////////////////////////////////////////////////////////////////
        // IMPLEMENTATION                                                            //
        ///////////////////////////////////////////////////////////////////////////////
        template <typename R, typename I>
        void ResourceManager<R, I>::make_resource(R* resource) {
            std::string path = conversionLongString(reinterpret_cast<unsigned long long int>(resource));
            if (!exists(path)) {
                std::unique_ptr<R> res;
                res.reset(resource);
                insertResource(path, std::move(resource));
            }
        }
        template <typename R, typename I>
        void ResourceManager<R, I>::make_resource(R* resource, const I& alias) {
            std::string path = conversionLongString(reinterpret_cast<unsigned long long int>(resource));
            if (!exists(path)) {
                std::unique_ptr<R> res;
                res.reset(resource);
                typename std::map<I, unsigned int>::iterator it = mAliasMap.find(alias);
                if (it == mAliasMap.end()) {
                    insertResource(path, std::move(resource));
                } else {
                    throw Erreur(5, "Alias already used!", 0);
                }
            }
        }
        template <typename R, typename I>
        template <typename... A>
        void ResourceManager<R, I>::fromFile(const std::string& path, A... args)
        {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            if (!exists(path)) {
                std::unique_ptr<R> resource = std::make_unique<R>();
                insertResource(path, std::move(resource));
            }
        }
        template <typename R, typename I>
        template <typename... A>
        void ResourceManager<R, I>::fromFileWithAlias(const std::string& path, const I& alias, A... args)
        {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            // Create and load resource
            if (!exists(path)) {
                std::unique_ptr<R> resource = std::make_unique<R>();
                typename std::map<I, unsigned int>::iterator it = mAliasMap.find(alias);
                if (it == mAliasMap.end()) {
                    if (!resource->loadFromFile(path, args...)) {
                        throw Erreur(6, "ResourceManager::load - Failed to load "+path,0);
                    }
                    // If loading successful, insert resource to map
                    insertResource(path, std::move(resource), alias);
                } else {
                    throw Erreur(5, "Alias already used!", 0);
                }
            }
            unsigned int id = getIdByPath(path);
            mAliasMap.insert(std::make_pair(alias, id));
        }
        template <typename R, typename I>
        template <typename... A, typename L>
        void ResourceManager<R, I>::fromMemory(L* localisation, A... args)
        {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            if (!exists(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)))) {
                std::unique_ptr<R> resource = std::make_unique<R>();
                if (!resource->loadFromMemory(localisation, args...)) {
                    throw Erreur(6, "ResourceManager::load - Failed to load ",0);
                }
                insertResource(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)), std::move(resource));
            }
        }
        template <typename R, typename I>
        template <typename... A, typename L>
        void ResourceManager<R, I>::fromMemoryWithAlias(L* localisation, const I& alias, A... args)
        {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            // Create and load resource
            if (!exists(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)))) {
                std::unique_ptr<R> resource = std::make_unique<R>();
                typename std::map<I, unsigned int>::iterator it = mAliasMap.find(alias);
                if (it == mAliasMap.end()) {
                    if (!resource->loadFromMemory(localisation, args...)) {
                        throw Erreur(6, "ResourceManager::load - Failed to load ",0);
                    }
                    // If loading successful, insert resource to map
                    insertResource(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)), std::move(resource), alias);
                } else {
                    throw Erreur(5, "Alias already used!", 0);
                }
            }
            unsigned int id = getIdByPath(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)));
            mAliasMap.insert(std::make_pair(alias, id));
        }
        template <typename R, typename I>
        template <typename... A>
        void ResourceManager<R, I>::fromFile (std::function<bool(R*,A...)> func, const std::string& path, A... args) {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            if (!exists(path)) {
                std::unique_ptr<R> resource = std::make_unique<R>();
                if (!func(resource, path, args...)) {
                    throw Erreur(6, "ResourceManager::load - Failed to load "+path,0);
                    delete resource;
                }
                insertResource(path, std::move(resource));
            }
        }
        template <typename R, typename I>
        template <typename... A>
        void ResourceManager<R, I>::fromFileWithAlias(std::function<bool(R*, A...)> func, const std::string& path, const I& alias, A... args)
        {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            if (!exists(path) == nullptr) {
                // Create and load resource
                std::unique_ptr<R> resource = std::make_unique<R>();
                typename std::map<I, unsigned int>::iterator it = mAliasMap.find(alias);
                if (it == mAliasMap.end()) {
                    if (!func(resource, path, args...)) {
                        throw Erreur(6, "ResourceManager::load - Failed to load ",0);
                    }
                    // If loading successful, insert resource to map
                    insertResource(path, std::move(resource), alias);
                } else {
                    throw Erreur(5, "Alias already used!", 0);
                }
            }
            unsigned int id = getIdByPath(path);
            mAliasMap.insert(std::make_pair(alias, id));
        }
        template <typename R, typename I>
        template <typename... A, typename L>
        void ResourceManager<R, I>::fromMemory (std::function<bool(R*,A...)> func, L* localisation, A... args) {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            if (!exists(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)))) {
                std::unique_ptr<R> resource = std::make_unique<R>();
                if (!func(resource, localisation, args...)) {
                    throw Erreur(6, "ResourceManager::load - Failed to load ",0);
                }
                insertResource(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)), resource);
            }
        }
        template <typename R, typename I>
        template <typename... A, typename L>
        void ResourceManager<R, I>::fromMemoryWithAlias(std::function<bool(R*, A...)> func, L* localisation, const I& alias, A... args)
        {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            if (!exists(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)))) {
                // Create and load resource
                std::unique_ptr<R> resource = std::make_unique<R>();
                typename std::map<I, unsigned int>::iterator it = mAliasMap.find(alias);
                if (it == mAliasMap.end()) {
                    if (!func(resource, localisation, args...)) {
                        throw Erreur(6, "ResourceManager::load - Failed to load ",0);
                    }
                    // If loading successful, insert resource to map
                    insertResource(conversionLongString(reinterpret_cast<unsigned long long int>(localisation)), std::move(resource), alias);
                } else {
                    throw Erreur(5, "Alias already used!", 0);
                }
            }
            unsigned int id = getIdByPath(reinterpret_cast<unsigned long long int>(localisation));
            mAliasMap.insert(std::make_pair(alias, id));
        }
        template <typename R, typename I>
        bool ResourceManager<R, I>::exists(const std::string &path) const
        {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            typename std::vector<Resource>::const_iterator it;
            for (it = mResourceMap.begin(); it != mResourceMap.end(); it++) {
                if (it->getPath() == path)
                    return true;
            }
            return false;
        }
        template <typename R, typename I>
        bool ResourceManager<R, I>::exists(const unsigned int id) const
        {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            return (id >= 0 && id < mResourceMap.size());
        }
        template <typename R, typename I>
        const unsigned int& ResourceManager<R, I>::getIdByAlias (const I &alias) const {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            typename std::map<I, unsigned int>::const_iterator it = mAliasMap.find(alias);
            if (it == mAliasMap.end())
                throw Erreur(8, "Alias not found!",0);
            return it->second;
        }
        template <typename R, typename I>
        const unsigned int& ResourceManager<R, I>::getIdByResource (const R* resource) const {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            typename std::vector<Resource>::const_iterator it;
            for (it = mResourceMap.begin(); it != mResourceMap.end(); it++) {
                if (it->getResource() == resource)
                    return it->getId();
            }
            throw Erreur(7, "Id not found!",0);
        }
        template <typename R, typename I>
        const unsigned int& ResourceManager<R, I>::getIdByPath (const std::string &path) const {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            typename std::vector<Resource>::const_iterator it;
            for (it = mResourceMap.begin(); it != mResourceMap.end(); it++) {
                if (it->getPath() == path)
                    return it->getId();
            }
            throw Erreur(7, "Path not found!",0);
        }
        template <typename R, typename I>
        const R* ResourceManager<R, I>::getResourceById (const unsigned int id) const
        {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            if(id >= mResourceMap.size())
                throw Erreur (7, "Id not found!", 0);
            return mResourceMap[id].getResource();
        }
        template <typename R, typename I>
        const R* ResourceManager<R, I>::getResourceByAlias(const I &alias) const
        {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            int id = getIdByAlias(alias);
            return getResourceById(id);
        }
        template <typename R, typename I>
        const R* ResourceManager<R, I>::getResourceByPath(const std::string &path) const
        {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            typename std::vector<Resource>::const_iterator it;
            for (it = mResourceMap.begin(); it != mResourceMap.end(); it++) {
                if (it->getPath() == path)
                    return it->getResource();
            }
            throw Erreur(7, "Path not found!",0);
        }
        template <typename R, typename I>
        std::string ResourceManager<R, I>::getPathById (const unsigned int id) const {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            typename std::vector<Resource>::iterator it;
            R* resource = getResourceById(id);
            if (resource != nullptr)
                return resource->getPath();
            throw Erreur (7, "Id not found!", 0);
        }
        template <typename R, typename I>
        std::string ResourceManager<R, I>::getPathByAlias (const I &alias) const {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            unsigned int id = getIdByAlias(alias);
            R* resource = getResourceById();
            return resource->getPath();
        }
        template <typename R, typename I>
        std::string ResourceManager<R, I>::getPathByResource (const R* resource) const {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            typename std::vector<Resource>::const_iterator it;
            for (it = mResourceMap.begin(); it != mResourceMap.end(); it++)
                if (it->getResource() == resource)
                    return it->getPath();
            throw new Erreur (7, "Resource not found!", 0);
        }
        template <typename R, typename I>
        std::vector<I> ResourceManager<R, I>::getAliasByResource(R* resource) {
            typename std::map<I, unsigned int>::iterator it;
            unsigned int id = getIdByResource(resource);
            typename std::vector<I> alias;
            for (it = mAliasMap.begin(); it != mAliasMap.end(); it++) {
                if (it->second == id)
                    alias.push_back(it->first);
            }
            return alias;
        }
        template <typename R, typename I>
        std::vector<std::string> ResourceManager<R, I>::getPaths () {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            std::vector<std::string> paths;
            typename std::vector<Resource>::iterator it;
            for (it = mResourceMap.begin(); it != mResourceMap.end(); it++) {
                  paths.push_back(it->getPath());
            }
            return paths;
        }
        template <typename R, typename I>
        void ResourceManager<R, I>::insertResource(const std::string &path, std::unique_ptr<R>&& resource, const I &alias)
        {
            Resource r(path, std::move(resource), nbResources);
            // Insert and check success
            mResourceMap.push_back(std::move(r));
            mAliasMap.insert(std::make_pair(alias, nbResources));
            nbResources++;
        }
        template <typename R, typename I>
        void ResourceManager<R, I>::insertResource(const std::string &path, std::unique_ptr<R>&& resource)
        {
            Resource r(path, std::move(resource), nbResources);
            // Insert and check success
            mResourceMap.push_back(std::move(r));
            nbResources++;
        }
        template <typename R, typename I>
        void ResourceManager<R, I>::deleteResourceById(const unsigned int id) {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            R* resource = const_cast<R*>(getResourceById(id));
            deleteResource(resource);
        }
        template <typename R, typename I>
        void ResourceManager<R, I>::deleteResourceByPath (const std::string &path) {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            R* resource = const_cast<R*>(getResourceByPath(path));
            deleteResource(resource);
        }
        template <typename R, typename I>
        void ResourceManager<R, I>::deleteResourceByAlias (const I &alias) {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            R* resource = const_cast<R*> (getResourceByAlias(alias));
            deleteResource(resource);
        }
        template <typename R, typename I>
        void ResourceManager<R, I>::deleteResource(R* resource) {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            typename std::vector<Resource>::iterator it;
            for (it = mResourceMap.begin(); it != mResourceMap.end();) {
                if (it->getResource() != nullptr && it->getResource() == resource) {
                    typename std::map<I, unsigned int>::iterator it2;
                    for (it2 = mAliasMap.begin(); it2 != mAliasMap.end();) {
                        if (it->getId() == it2->second)
                            mAliasMap.erase(it2);
                        else
                            it2++;
                    }
                    R* res = const_cast<R*>(it->getResource());
                    delete res;
                    res = nullptr;
                } else {
                    it++;
                }
            }
        }
        template <typename R, typename I>
        void ResourceManager<R, I>::deleteAll() {
            std::lock_guard<std::recursive_mutex> locker(rec_mutex);
            typename std::vector<Resource>::iterator it;
            for (it = mResourceMap.begin(); it != mResourceMap.end();) {
                if (it->getResource() != nullptr) {
                    R* resource = const_cast<R*>(it->getResource());
                    delete resource;
                    resource = nullptr;
                } else {
                    it++;
                }
            }
            mResourceMap.clear();
            mAliasMap.clear();
        }
        template <typename R, typename I>
        std::unique_ptr<ResourceManagerBase> ResourceManager<R, I>::clone() {
            return std::make_unique<ResourceManager<R, I>>(*this);
        }
        template <typename B, typename I = std::string>
        class BaseResourceManager : public ResourceManager<B, I> {
            std::unique_ptr<ResourceManagerBase> clone() {
                return std::make_unique<BaseResourceManager<B, I>>(*this);
            }
        };
    }
}
namespace odfaeg {
    namespace graphic {
        class Texture;
        class Font;
        class Shader;
    }
}
namespace sf {
    class SoundBuffer;
}
////////////////
namespace odfaeg {
    namespace core {
        template <typename I=std::string> using TextureManager = ResourceManager<odfaeg::graphic::Texture, I>;
        template <typename I=std::string> using ShaderManager = ResourceManager<odfaeg::graphic::Shader, I>;
        template <typename I=std::string> using FontManager = ResourceManager<odfaeg::graphic::Font, I>;
        template <typename I=std::string> using SoundBufferManager = ResourceManager<sf::SoundBuffer, I>;
    }
}

#endif // RESOURCEMANAGER_HPP_INCLUDED

#ifndef ENTITY_FACTORY_HPP
#define ENTITY_FACTORY_HPP
#include <entt.hpp>
namespace odfaeg {
    namespace graphic {
        using EntityId = entt::entity;
        class ODFAEG_GRAPHICS_API EntityFactory {
            public :

                EntityFactory() {
                    nbEntities = 0;
                    nbEntitiesTypes = 0;
                }
                int getIntOfType(std::string sType) {
                    std::map<int, std::string>::iterator it;
                    for (it = types.begin(); it != types.end(); ++it) {
                        if (it->second == sType) {
                            return it->first;
                        }
                    }
                    return -1;
                }
                std::pair<int, std::string> updateTypes(std::string sType) {
                    int iType = getIntOfType(sType);
                    /*if (sType == "E_WALL")
                        //////std::cout<<"i type : "<<iType<<std::endl;*/
                    if (iType == -1) {
                        /*if (sType == "E_WALL")
                            //////std::cout<<"i type  : "<<nbEntitiesTypes<<std::endl;*/
                        std::pair<int, std::string> type = std::pair<int, std::string> (nbEntitiesTypes, sType);
                        types.insert(type);
                        nbEntitiesTypes++;
                        return type;
                    } else {
                        std::map<int, std::string>::iterator it = types.find(iType);
                        /*if (it->second == "E_WALL")
                            //////std::cout<<"i type : "<<it->first<<std::endl;*/
                        return *it;
                    }
                }
                std::string getTypeOfInt (int type) {
                    std::map<int, std::string>::iterator it = types.find(type);
                    return it != types.end() ? it->second : "";
                }
                int getNbEntitiesTypes () {
                    return nbEntitiesTypes;
                }
                int getNbEntities () {
                    return nbEntities;
                }
                void decreaseNbEntities() {
                    nbEntities--;
                }
                template <typename D, typename... Args>
                D* make_entity(Args&&... args) {
                    return new D(std::forward<Args>(args)...);
                }
                int getUniqueId() {
                    nbEntities++;
                    return nbEntities-1;
                }
                EntityId getEnttID() {
                    return m_Registery.create();
                }
                entt::registry& getRegistery() {
                    return m_Registery;
                }
            private :
                EntityFactory(const EntityFactory& entity) = delete; /**> an entity if not copiable.*/
                EntityFactory& operator=(const EntityFactory& entity) = delete; /**> an entity is not affectable*/
                unsigned int nbEntities, nbEntitiesTypes;
                std::map<int, std::string> types;
                entt::registry m_Registery;

        };
    }
}
#endif

#ifndef ENTITY_FACTORY_HPP
#define ENTITY_FACTORY_HPP
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            using EntityId = std::size_t;
            struct EntityFactory {
                EntityFactory() {
                    nbEntitiesTypes = 0;
                    nbEntities = 0;
                }
                EntityId createEntity(std::string groupName="") {
                    EntityId id;
                    id = nbEntities;
                    nbEntities++;
                    if (groupName != "") {
                        updateTypes(groupName);
                    }
                    return id;
                }
                std::size_t getNbEntities() {
                    return nbEntities;
                }
                std::size_t getNbEntitiesTypes() {
                    return nbEntitiesTypes;
                }
                int getIntOfType(std::string sType) {
                    std::map<int, std::string>::iterator it;
                    for (it = types.begin(); it != types.end(); ++it) {
                        if (it->second == sType)
                            return it->first;
                    }
                    return -1;
                }
                std::pair<int, std::string> updateTypes(std::string sType) {
                    int iType = getIntOfType(sType);
                    if (iType == -1) {
                        std::pair<int, std::string> type = std::pair<int, std::string> (nbEntitiesTypes, sType);
                        types.insert(type);
                        nbEntitiesTypes++;
                        return type;
                    } else {
                        std::map<int, std::string>::iterator it = types.find(iType);
                        return *it;
                    }
                }
                std::string getTypeOfInt (int type) {
                    std::map<int, std::string>::iterator it = types.find(type);
                    return it != types.end() ? it->second : "";
                }
                private :
                std::size_t nbEntities, nbEntitiesTypes;
                std::map<int, std::string> types;
            };
        }
    }
}
#endif

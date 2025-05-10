#include "../../../../include/odfaeg/Graphics/ECS/systems.hpp"
size_t getGroupId (std::string groupName) {
    static size_t nbGroups=0;
    static std::map<std::string, size_t> mapping;
    static std::map<std::string, size_t>::iterator it = mapping.find(groupName);
    if (it == mapping.end()) {
        size_t groupId = nbGroups;
        mapping.insert(std::make_pair(groupName, groupId));
        nbGroups++;
        return groupId;
    }
    return it->second;
}

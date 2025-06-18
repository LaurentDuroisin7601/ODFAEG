#include "quest.hpp"
#include <vector>
#include "pnj.hpp"
namespace sorrok {
    using namespace odfaeg::graphic;
    Quest::Quest() {
        name="";
        task="";
        xp = 0;
        status = NEW;
        pnjToVisit = "";
        repeat = false;
    }
    Quest::Quest(std::string name, std::string task, bool repeat) : name(name), task(task), repeat(repeat) {
        xp = 0;
        status = NEW;
        pnjToVisit = "";
    }
    std::string Quest::getName() {
        return name;
    }
    std::string Quest::getTask() {
        return task;
    }
    void Quest::addItemToCollect(std::string name, unsigned int nb) {
        std::map<std::string, std::pair<unsigned int, unsigned int>>::iterator it;
        bool found = false;
        for (it = itemsToCollect.begin(); it != itemsToCollect.end() && !found; it++) {
            if (it->first == name) {
                found = true;
            }
        }
        if (!found)
            itemsToCollect.insert(std::make_pair(name, std::make_pair(nb, 0)));
    }
    void Quest::addMonsterToKill(std::string name, unsigned int nb) {
        std::map<std::string, std::pair<unsigned int, unsigned int>>::iterator it;
        bool found = false;
        for (it = monstersToKill.begin(); it != monstersToKill.end() && !found; it++) {
            if (it->first == name) {
                found = true;
            }
        }
        if (!found)
            monstersToKill.insert(std::make_pair(name, std::make_pair(nb, 0)));
    }
    void Quest::addItemCollectedProgress(std::string name) {
        std::map<std::string, std::pair<unsigned int, unsigned int>>::iterator it;
        it = itemsToCollect.find(name);
        if (it != itemsToCollect.end() && it->second.second < it->second.first) {
            it->second.second++;
            if (checkIfIsComplete()) {
                status = COMPLETE;
            }
        }
    }
    void Quest::addMonsterToKillProgress(std::string name) {
        std::map<std::string, std::pair<unsigned int, unsigned int>>::iterator it;
        it = monstersToKill.find(name);
        if (it != monstersToKill.end() && it->second.second < it->second.first) {
            it->second.second++;
            if (checkIfIsComplete()) {
                status = COMPLETE;
            }
        }
    }
    bool Quest::checkIfIsComplete() {
        std::map<std::string, std::pair<unsigned int, unsigned int>>::iterator it;
        for (it = monstersToKill.begin(); it != monstersToKill.end(); it++) {
            if (it->second.first != it->second.second)
                return false;
        }
        for (it = itemsToCollect.begin(); it != itemsToCollect.end(); it++) {
            if (it->second.first != it->second.second)
                return false;
        }
        return true;
    }
    std::map<std::string, std::pair<unsigned int, unsigned int>> Quest::getMonsterToKill() {
        return monstersToKill;
    }
    std::map<std::string, std::pair<unsigned int, unsigned int>> Quest::getItemsToCollect() {
        return itemsToCollect;
    }
    void Quest::setPnjToVisit(std::string name) {
        pnjToVisit = name;
    }
    std::string Quest::getPnjToVisit() {
        return pnjToVisit;
    }
    void Quest::setXp (int xp) {
        this->xp = xp;
    }
    int Quest::getXp() {
        return xp;
    }
    void Quest::addReward(unsigned int nb, Item item) {
        bool found = false;
        std::map<unsigned int, Item>::iterator it;
        for (it = rewards.begin(); it != rewards.end() && !found; it++) {
            if (it->second.getName() == item.getName()) {
                found = true;
            }
        }
        if (!found)
            rewards.insert(std::make_pair(nb, item));
    }
    std::map<unsigned int, Item> Quest::getRewards() {
        return rewards;
    }
    void Quest::setStatus(Status status) {
        this->status = status;
    }
    Quest::Status Quest::getStatus() {
        return status;
    }
    bool Quest::isRepeat() {
        return repeat;
    }
    void Quest::reset() {
        std::map<std::string, std::pair<unsigned int, unsigned int>>::iterator it;
        for (it = monstersToKill.begin(); it != monstersToKill.end(); it++) {
            it->second.second = 0;
        }
        for (it = itemsToCollect.begin(); it != itemsToCollect.end(); it++) {
            it->second.second = 0;
        }
    }
}

#include "quest.hpp"
namespace sorrok {
    Quest::Quest() {
        name = "";
        task = "";
        xp = 0;
        status = NEW;
        pnjToVisit = "";
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
        itemsToCollect.insert(std::make_pair(name, std::make_pair(nb, 0)));
    }
    void Quest::addMonsterToKill(std::string name, unsigned int nb) {
        monstersToKill.insert(std::make_pair(name, std::make_pair(nb, 0)));
    }
    void Quest::addItemCollectedProgress(std::string name) {
        std::map<std::string, std::pair<unsigned int, unsigned int>>::iterator it;
        it = itemsToCollect.find(name);
        if (it != itemsToCollect.end() && it->second.second < it->second.first) {
            it->second.second++;
        }
    }
    void Quest::addMonsterToKillProgress(std::string name) {
        std::map<std::string, std::pair<unsigned int, unsigned int>>::iterator it;
        it = monstersToKill.find(name);
        if (it != monstersToKill.end() && it->second.second < it->second.first) {
            it->second.second++;
        }
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
}

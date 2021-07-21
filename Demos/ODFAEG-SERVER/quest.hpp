#ifndef QUEST_HPP
#define QUEST_HPP
#include <string>
#include <map>
#include "item.hpp"
#include <vector>
#include <iostream>
namespace sorrok {
    class Quest {
        enum Status {
            NEW, IN_PROGRESS, COMPLETE, FINISHED
        };
        public :
            Quest();
            Quest (std::string name, std::string task, bool repeat);
            std::string getName();
            std::string getTask();
            void addItemToCollect(std::string, unsigned int nb);
            void addMonsterToKill(std::string, unsigned int nb);
            bool isMonsterToKillInList(std::string name);
            bool isItemToCollectInList(std::string name);
            void addItemCollectedProgress(std::string name);
            void addMonsterToKillProgress(std::string name);
            void setPnjToVisit(std::string name);
            void setXp (int xp);
            void addReward(unsigned int, Item item);
            std::map<unsigned int, Item> getRewards();
            std::string getPnjToVisit();
            int getXp();
            void setStatus (Status status);
            Status getStatus();
            bool isRepeat();
            template <typename Archive>
            void serialize (Archive &ar) {
                ar(name);
                ar(task);
                ar(itemsToCollect);
                ar(monstersToKill);
                ar(pnjToVisit);
                ar(rewards);
                ar(status);
                ar(repeat);
            }
        private :
            std::string name, pnjToVisit;
            std::string task;
            std::map<std::string, std::pair<unsigned int, unsigned int>> itemsToCollect;
            std::map<std::string, std::pair<unsigned int, unsigned int>> monstersToKill;
            int xp;
            std::map<unsigned int, Item> rewards;
            Status status;
            bool repeat;
    };
}
#endif

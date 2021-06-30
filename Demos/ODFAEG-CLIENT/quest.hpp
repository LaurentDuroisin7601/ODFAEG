#ifndef QUEST_HPP
#define QUEST_HPP
#include <string>
#include <map>
#include "item.hpp"
#include <map>
#include <iostream>
namespace sorrok {
    class Quest {
        public :
            enum Status {
                NEW, IN_PROGRESS, COMPLETE, FINISHED
            };
            Quest ();
            Quest (std::string name, std::string task, bool repeat);
            std::string getName();
            std::string getTask();
            void addItemToCollect(std::string name, unsigned int nb);
            void addMonsterToKill(std::string name, unsigned int nb);
            bool isMonsterToKillInList(std::string name);
            bool isItemToCollectInList(std::string name);
            void addItemCollectedProgress(std::string name);
            void addMonsterToKillProgress(std::string name);
            std::map<std::string, std::pair<unsigned int, unsigned int>> getMonsterToKill();
            std::map<std::string, std::pair<unsigned int, unsigned int>> getItemsToCollect();
            void setPnjToVisit(std::string name);
            std::string getPnjToVisit();
            void setXp (int xp);
            int getXp();
            void addReward (unsigned int nb, Item item);
            void reset();
            std::map<unsigned int, Item> getRewards();
            void setStatus (Status status);
            Status getStatus();
            bool isRepeat();
            template <typename Archive>
            void serialize(Archive &ar) {
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
            bool checkIfIsComplete();
            bool repeat;
            std::string name, pnjToVisit;
            std::string task;
            std::map<std::string, std::pair<unsigned int, unsigned int>> itemsToCollect;
            std::map<std::string, std::pair<unsigned int, unsigned int>> monstersToKill;
            int xp;
            std::map<unsigned int, Item> rewards;
            Status status;
    };
}
#endif

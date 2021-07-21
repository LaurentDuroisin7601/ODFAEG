#ifndef SORROK_PNJ
#define SORROK_PNJ
#include "quest.hpp"
#include "caracter.h"
#include <vector>
namespace sorrok {
    class Pnj : public Caracter {
        public :
            Pnj();
            bool isMovingFromKeyboard();
            bool isMonster();
            void addQuest(Quest quest);
            template <typename Archive>
            void vtserialize(Archive& ar) {
                Caracter::vtserialize(ar);
                ar(quests);
            }
            std::vector<Quest>& getQuests();
        private :
            std::vector<Quest> quests;
    };
}
#endif

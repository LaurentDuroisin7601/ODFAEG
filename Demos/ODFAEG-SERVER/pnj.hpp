#ifndef SORROK_PNJ
#define SORROK_PNJ
#include "caracter.h"
#include <vector>
#include "quest.hpp"
namespace sorrok {
    class Pnj : public Caracter {
        public :
            Pnj(odfaeg::graphic::EntityFactory& factory);
            void addQuest(Quest quest);
            std::vector<Quest> getQuests();
            bool isMovingFromKeyboard();
            bool isMonster();
            template <typename Archive>
            void vtserialize(Archive &ar) {
                Caracter::vtserialize(ar);
                ar(quests);
            }
            odfaeg::graphic::Entity* clone();
        private :
            std::vector<Quest> quests;
    };
}
#endif

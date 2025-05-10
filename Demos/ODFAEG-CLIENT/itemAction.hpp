#ifndef ITEM_ACTION_HPP
#define ITEM_ACTION_HPP
#include "hero.h"
#include "odfaeg/Network/network.h"
#include "odfaeg/Core/utilities.h"
namespace sorrok {
    class ItemAction {
        public :
        void useHpPotion(Hero* hero, Item hpPotion);
    };
}
#endif

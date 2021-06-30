#ifndef SORROK_SKILL_ACTION_HPP
#define SORROK_SKILL_ACTION_HPP
#include "hero.h"
#include "odfaeg/Network/network.h"
#include "odfaeg/Core/utilities.h"
namespace sorrok {
    class SkillAction {
    public :
        void launchLastHeal(Hero* hero, Skill skill);
    };
}
#endif

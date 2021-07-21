#ifndef ODFAEG_PONG_CLIENT
#define ODFAEG_PONG_CLIENT
#include "odfaeg/Core/application.h"
#include "odfaeg/Network/network.h"
#include "partie.hpp"
#include "odfaeg/Graphics/world.h"
#include "odfaeg/Graphics/entitiesUpdater.h"
#include "odfaeg/Graphics/map.h"
#include "bar.hpp"
#include "odfaeg/Graphics/GUI/button.hpp"
#include "odfaeg/Graphics/GUI/textArea.hpp"
#include "odfaeg/Graphics/GUI/optionPane.hpp"
class Pong : public odfaeg::core::Application, public odfaeg::graphic::gui::ActionListener {
    public :
        Pong();
        void onKeyPressed (sf::Keyboard::Key key);
        void onKeyReleased (sf::Keyboard::Key key);
        void onLoad();
        void onInit();
        void onRender(odfaeg::graphic::FastRenderComponentManager* frcm);
        void onDisplay(odfaeg::graphic::RenderWindow* window);
        void onUpdate(sf::Event& event);
        void onExec();
        void actionPerformed(odfaeg::graphic::gui::Button* button);
    private :
        odfaeg::graphic::gui::TextArea* m_taPseudo;
        odfaeg::graphic::gui::Button*  m_bPseudo;
        odfaeg::graphic::gui::OptionPane *m_pseudoUsed, *m_confirmInvitation;
        std::vector<odfaeg::graphic::gui::Button*> m_bPseudos;
        odfaeg::core::ResourceCache<> cache;
        Player* m_player1;
        Player* m_player2;
        Partie* m_party;
        Ball* m_ball;
        Bar *top, *bottom;
        sf::Time timeBtwnTwoUpdates;
        sf::Int64 ping;
        int maxI;
};
#endif // ODFAEG_PONG_CLIENT

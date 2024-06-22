#ifndef TAB_PANE
#define TAB_PANE
#include "../lightComponent.h"
#include "panel.hpp"
#include "label.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class TabPane : public LightComponent {
                public :
                    TabPane(RenderWindow& window, math::Vec3f position, math::Vec3f size);
                    void addTab(Panel* panel, std::string tabName, const Font& font);
                    void clear() {
                        for (unsigned int i = 0; i < getChildren().size(); i++)
                            getChildren()[i]->clear();
                    }
                    void drawOn(RenderTarget& target, RenderStates states);
                    void onDraw(RenderTarget& target, RenderStates states);
                    std::string getSelectedTab();

                private :
                    bool isOnXScroll();
                    void moveXItems();
                    void onTabChanged(Panel* panel);
                    bool scrollX;
                    int mouseDeltaX, oldMouseX, maxSizeX;
                    odfaeg::graphic::RectangleShape vertScrollBar;
                    Label* selectedTab;
            };
        }
    }
}
#endif

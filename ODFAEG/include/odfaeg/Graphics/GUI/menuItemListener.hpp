#ifndef MENU_ITEM_LISTENER_HPP
#define MENU_ITEM_LISTENER_HPP
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class MenuItem;
            class ODFAEG_GRAPHICS_API MenuItemListener {
                public :
                virtual void actionPerformed(MenuItem* menuItem) = 0;
            };
        }
    }
}
#endif

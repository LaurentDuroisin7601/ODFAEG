#ifndef ODFAEG_ACTION_LISTENER_HPP
#define ODFAEG_ACTION_LISTENER_HPP
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class Button;
            class ODFAEG_GRAPHICS_API ActionListener {
                public :
                virtual void actionPerformed(Button* button) = 0;
            };
        }
    }
}
#endif // ODFAEG_ACTION_LISTENER_HPP

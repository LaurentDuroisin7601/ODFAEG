#ifndef ODFAEG_BILLBOARD_HPP
#define ODFAEG_BILLBOARD_HPP
#include "entity.h"
#include "view.h"
namespace odfaeg {
    namespace graphic {
        class BillBoard {
        public :
            BillBoard(View view, Entity& drawable);
            View getView();
            void setView (View view);
        private :
            View view;
            Entity& entity;
        };
    }
}
#endif // ODFAEG_BILLBOARD_HPP

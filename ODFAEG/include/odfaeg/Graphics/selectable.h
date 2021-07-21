#ifndef ODFAEG_SELECTABLE_2D_HPP
#define ODFAEG_SELECTABLE_2D_HPP
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
namespace graphic {
        /**
        * \file selectable.h
        * \class Selectable
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        * \brief represent a selectable objet
        */
        class ODFAEG_GRAPHICS_API Selectable {
        public :
            /**
            * \fn void setSelected(bool b);
            * \brief change the state of the selected object.
            * \param b : the state of the selected object.
            */
            void setSelected(bool b) {
                if (selected != b && b)
                    onSelect();
                else if (selected != b && !b)
                    onDeselect();
                selected = b;
            }
            /**
            * \fn isSelected()
            * \brief return if the object is selected.
            * \return if the object is selected.
            */
            bool isSelected() {
                return selected;
            }
            /**
            * \fn void onSelect ()
            * \brief this function can be redefined to do something when a object is selected.
            */
            virtual void onSelect () { }
            /**
            * \fn void onSelect ()
            * \brief this function can be redefined to do something when a object is deselected.
            */
            virtual void onDeselect() { }
        private :
            bool selected; /**> a boolean which holds if the state of the entity. (selected or deselected) */
        };
    }
}
#endif // SELECTABLE

#ifndef ODFAEG_CREATOR_RECTANGULAR_SELECTION_HPP
#define ODFAEG_CREATOR_RECTANGULAR_SELECTION_HPP
#include "odfaeg/Graphics/rectangleShape.h"
class RectangularSelection : public odfaeg::graphic::Drawable {
public :
    RectangularSelection();
    void setRect(int posX, int posY, int posZ, int width, int height, int depth);
    void addItem(odfaeg::graphic::Transformable* item);
    odfaeg::physic::BoundingBox getSelectionRect();
    std::vector<odfaeg::graphic::Transformable*>& getItems();
    void setColor(odfaeg::graphic::Transformable* transformable);
    void draw (odfaeg::graphic::RenderTarget& target, odfaeg::graphic::RenderStates states);
private :
    odfaeg::graphic::RectangleShape selectionRect;
    std::vector<odfaeg::graphic::Transformable*> items;
};
#endif // RECTANGULAR_SELECTION

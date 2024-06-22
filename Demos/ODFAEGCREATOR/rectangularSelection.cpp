#include "rectangularSelection.hpp"
#include "odfaeg/Graphics/renderTarget.h"
using namespace odfaeg::graphic;
using namespace odfaeg::math;
using namespace odfaeg::physic;
RectangularSelection::RectangularSelection() {
    selectionRect.setOutlineThickness(1.f);
    selectionRect.setFillColor(sf::Color::Transparent);
    selectionRect.setOutlineColor(sf::Color::Red);
}
void RectangularSelection::setRect(int posX, int posY, int posZ, int width, int height, int depth) {
    selectionRect.setSize(Vec3f(width, height, depth));
    selectionRect.setPosition(Vec3f(posX, posY, posZ));
    //std::cout<<"select rect size : "<<selectionRect.getSize()<<std::endl;
}
void RectangularSelection::addItem(Transformable* item) {
    bool contains = false;
    for (unsigned int i = 0; i < items.size() && !contains; i++) {
        if (items[i] == item)
            contains = true;
    }
    if (!contains)
        items.push_back(item);
}
BoundingBox RectangularSelection::getSelectionRect() {
    BoundingBox rect(selectionRect.getPosition().x, selectionRect.getPosition().y, selectionRect.getPosition().z, selectionRect.getSize().x, selectionRect.getSize().y, selectionRect.getSize().z);
    return rect;
}
std::vector<Transformable*>& RectangularSelection::getItems() {
    return items;
}
void RectangularSelection::draw(RenderTarget& target, RenderStates states) {
    target.draw(selectionRect, states);
}

#include "translationGismo.hpp"
using namespace odfaeg::graphic;
using namespace odfaeg::physic;
using namespace odfaeg::math;
TranslationGuismo::TranslationGuismo(float arrowSize) : Drawable(), Transformable(), arrowSize(arrowSize), visible(false) {

}
void TranslationGuismo::setCenterSize(Vec3f center, Vec3f size) {
    zRect = RectangleShape(Vec3f(0, size.y + arrowSize - size.y, size.z));
    zRect.setCenter(Vec3f(center.x, center.y - arrowSize * 0.5f, center.z));
    zRect.setFillColor(sf::Color::Blue);
    zArrow = ConvexShape(3);
    zArrow.setPoint(0, sf::Vector3f(center.x, center.y + arrowSize, center.z + size.z));
    zArrow.setPoint(1, sf::Vector3f(center.x, center.y, center.z + size.z + arrowSize));
    zArrow.setPoint(2, sf::Vector3f(center.x, center.y - arrowSize, center.z + size.z));
    zArrow.setFillColor(sf::Color::Blue);

    bpZArrow = BoundingPolyhedron(Vec3f(center.x,  center.y + arrowSize, center.z + size.z),
                            Vec3f(center.x,  center.y, center.z + size.z + arrowSize),
                            Vec3f(center.x,  center.y - arrowSize, center.z + size.z), true);
    xRect = RectangleShape(Vec3f(size.x, size.y + arrowSize - size.y, 0));
    xRect.setCenter(Vec3f(center.x, center.y - arrowSize * 0.5f, center.z));
    xRect.setFillColor(sf::Color::Red);
    xArrow = ConvexShape(3);
    xArrow.setPoint(0, sf::Vector3f(center.x + size.x, center.y + arrowSize, center.z));
    xArrow.setPoint(1, sf::Vector3f(center.x + size.x + arrowSize, center.y, center.z));
    xArrow.setPoint(2, sf::Vector3f(center.x + size.x, center.y - arrowSize, center.z));
    xArrow.setFillColor(sf::Color::Red);

    bpXArrow = BoundingPolyhedron(Vec3f(center.x + size.x,  center.y - arrowSize, center.z),
                            Vec3f(center.x + size.x + arrowSize,  center.y, center.z),
                            Vec3f(center.x + size.x,  center.y + arrowSize, center.z), true);

    yRect = RectangleShape(Vec3f(size.x + arrowSize - size.x, size.y, 0));
    yRect.setCenter(Vec3f(center.x - arrowSize * 0.5f, center.y, 0));
    yRect.setFillColor(sf::Color::Green);
    yArrow = ConvexShape(3);
    yArrow.setPoint(0, sf::Vector3f(center.x + arrowSize, center.y + size.y, center.z));
    yArrow.setPoint(1, sf::Vector3f(center.x, center.y + size.y + arrowSize, center.z));
    yArrow.setPoint(2, sf::Vector3f(center.x - arrowSize, center.y + size.y, center.z));
    yArrow.setFillColor(sf::Color::Green);

    bpYArrow = BoundingPolyhedron(Vec3f(center.x + arrowSize,  center.y + size.y, center.z),
                            Vec3f(center.x,  center.y + size.y + arrowSize, center.z),
                            Vec3f(center.x - arrowSize,  center.y + size.y, center.z), true);
}
void TranslationGuismo::draw(RenderTarget& target, RenderStates states) {
    if (visible) {
        target.draw(zRect);
        target.draw(zArrow);
        target.draw(xRect);
        target.draw(xArrow);
        target.draw(yRect);
        target.draw(yArrow);
    }
}
bool TranslationGuismo::intersectsXArrow(Ray ray, Vec3f& i1) {
    if (visible) {
        CollisionResultSet::Info info;
        Vec3f i2;
        return bpXArrow.intersectsWhere(ray, i1, i2, info);
    }
    return false;
}
bool TranslationGuismo::intersectsYArrow(Ray ray, Vec3f& i1) {
    if (visible) {
        CollisionResultSet::Info info;
        Vec3f i2;
        return bpYArrow.intersectsWhere(ray, i1, i2, info);
    }
    return false;
}
bool TranslationGuismo::intersectsZArrow(Ray ray, Vec3f& i1) {
    if (visible) {
        CollisionResultSet::Info info;
        Vec3f i2;
        return bpZArrow.intersectsWhere(ray, i1, i2, info);
    }
    return false;
}
void TranslationGuismo::setVisible(bool visible) {
    this->visible = visible;
}
void TranslationGuismo::move(Vec3f t) {
    zRect.move(t);
    zArrow.move(t);
    xRect.move(t);
    xArrow.move(t);
    yRect.move(t);
    yArrow.move(t);
    bpXArrow.move(t);
    bpYArrow.move(t);
    bpZArrow.move(t);
}


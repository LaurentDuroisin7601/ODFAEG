#include "scaleGismo.hpp"
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::graphic;
ScaleGuismo::ScaleGuismo(float rectSize) : Drawable(), Transformable(), rectSize(rectSize), visible(false) {
}
void ScaleGuismo::setCenterSize(Vec3f center, Vec3f size) {
    zRect = RectangleShape(Vec3f(0, size.y + rectSize - size.y, size.z));
    zRect.setCenter(Vec3f(center.x, center.y - rectSize * 0.5f, center.z));
    zRect.setFillColor(sf::Color::Blue);
    zScaleRect = RectangleShape(Vec3f(0, rectSize*2, rectSize*2));
    zScaleRect.setCenter(Vec3f(center.x, center.y - rectSize, center.z + size.z));
    zScaleRect.setFillColor(sf::Color::Blue);
    bbZScaleRect = BoundingBox(center.x, center.y - rectSize, center.z + size.z, 0, rectSize*2, rectSize*2);

    xRect = RectangleShape(Vec3f(size.x, size.y + rectSize - size.y, 0));
    xRect.setCenter(Vec3f(center.x, center.y - rectSize * 0.5f, center.z));
    xRect.setFillColor(sf::Color::Red);
    xScaleRect = RectangleShape(Vec3f(rectSize*2, rectSize*2, 0));
    xScaleRect.setCenter(Vec3f(center.x + size.x, center.y - rectSize, center.z));
    xScaleRect.setFillColor(sf::Color::Red);
    bbXScaleRect = BoundingBox(center.x + size.x, center.y - rectSize, center.z, rectSize*2, rectSize*2, 0);

    yRect = RectangleShape(Vec3f(size.x + rectSize - size.x, size.y, 0));
    yRect.setCenter(Vec3f(center.x - rectSize * 0.5f, center.y, 0));
    yRect.setFillColor(sf::Color::Green);
    yScaleRect = RectangleShape(Vec3f(rectSize*2, rectSize*2, 0));
    yScaleRect.setCenter(Vec3f(center.x - rectSize, center.y + size.y, center.z));
    yScaleRect.setFillColor(sf::Color::Green);
    bbYScaleRect = BoundingBox(center.x - rectSize, center.y + size.y, center.z, rectSize*2, rectSize*2, 0);
}
void ScaleGuismo::draw(RenderTarget& target, RenderStates states) {
    if (visible) {
        target.draw(xRect);
        target.draw(xScaleRect);
        target.draw(yRect);
        target.draw(yScaleRect);
        target.draw(zRect);
        target.draw(zScaleRect);
    }
}
bool ScaleGuismo::intersectsXRect(Ray ray) {
    if (visible) {
        CollisionResultSet::Info info;
        return bbXScaleRect.intersects(ray, true, info);
    }
    return false;
}
bool ScaleGuismo::intersectsYRect(Ray ray) {
    if (visible) {
        CollisionResultSet::Info info;
        return bbYScaleRect.intersects(ray, true, info);
    }
    return false;
}
bool ScaleGuismo::intersectsZRect(Ray ray) {
    if (visible) {
        CollisionResultSet::Info info;
        return bbZScaleRect.intersects(ray, true, info);
    }
    return false;
}
void ScaleGuismo::scale(Vec3f s) {
    if (s.x != 1 && s.y == 1 && s.z == 1) {
        bbXScaleRect.scale(s);
        xScaleRect.scale(s);
    } else if (s.x == 1 && s.y != 1 && s.z == 1) {
        bbYScaleRect.scale(s);
        yScaleRect.scale(s);
    } else {
        bbZScaleRect.scale(s);
        zScaleRect.scale(s);
    }
}
void ScaleGuismo::setVisible (bool visible) {
    this->visible = visible;
}

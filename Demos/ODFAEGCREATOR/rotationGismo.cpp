#include "rotationGismo.hpp"
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::graphic;
RotationGuismo::RotationGuismo() : Drawable(), Transformable(), visible(false) {
}
void RotationGuismo::setCenterSize(Vec3f center, Vec3f size) {
    float maxSize = size.x();
    if (size.y() > maxSize)
        maxSize = size.y();
    if (size.z() > maxSize)
        maxSize = size.z();
    cc = CircleShape(maxSize*0.5f);
    cc.setFillColor(Color::Transparent);
    cc.setOutlineThickness(2.f);
    cc.setOutlineColor(Color::Blue);
    bc = BoundingCircle(center, maxSize * 0.5f, Vec3f::zAxis);
    demiCircle = DemiCircle(maxSize*0.5f, 0, Color(128, 128, 128, 255));
    cc.setOrigin(Vec3f(maxSize*0.5f, maxSize*0.5f, maxSize*0.5f));
    cc.setCenter(center);
    demiCircle.setCenter(center);

}
void RotationGuismo::setAngle(Vec3f axis, float angle) {
    Vec3f xAxis (1, 0, 0);
    float a = xAxis.getAngleBetween(axis, xAxis.cross(axis));
    demiCircle.setRotation(Math::toDegrees(a)+180, axis.cross(xAxis));
    demiCircle.setAngle(angle);
}
bool RotationGuismo::isMouseOnCircle(Ray ray) {
    if (visible) {
        return bc.intersects(ray);
    }
    return false;
}
bool RotationGuismo::intersectsWhere(Vec3f& near, Ray ray) {
    if (visible) {
        return bc.intersectsWhere(ray, near);
    }
    return false;
}
void RotationGuismo::setVisible(bool visible) {
    this->visible = visible;
}
void RotationGuismo::draw(RenderTarget& target, RenderStates states) {
    if (visible) {
        target.draw(cc, states);
        target.draw(demiCircle, states);
    }
}
Vec3f RotationGuismo::getCenter() {
    return cc.getCenter();
}


#ifndef ROTATION_GUISMO_HPP
#define ROTATION_GUISMO_HPP
#include "odfaeg/Graphics/convexShape.h"
#include "odfaeg/Graphics/rectangleShape.h"
#include "odfaeg/Graphics/circleShape.h"
#include "odfaeg/Graphics/demiCircle.hpp"
#include "odfaeg/Physics/boundingCircle.hpp"
#include "odfaeg/Physics/boundingPolyhedron.h"
#include "odfaeg/Graphics/renderTarget.h"
class RotationGuismo : public odfaeg::graphic::Drawable, odfaeg::graphic::Transformable {
public :
    RotationGuismo ();
    void draw(odfaeg::graphic::RenderTarget& target, odfaeg::graphic::RenderStates states);
    void setAngle (odfaeg::math::Vec3f axis, float angle);
    bool isMouseOnCircle(odfaeg::math::Ray ray);
    bool intersectsWhere(odfaeg::math::Vec3f& near, odfaeg::math::Ray ray);
    void setVisible(bool visible);
    void setCenterSize(odfaeg::math::Vec3f center, odfaeg::math::Vec3f size);
    odfaeg::math::Vec3f getCenter();
private :
    odfaeg::physic::BoundingCircle bc;
    odfaeg::graphic::CircleShape cc;
    odfaeg::graphic::DemiCircle demiCircle;
    float arrowSize;
    bool visible;
};
#endif // ROTATION_GUISMO_HPP

#ifndef TRANSLATION_GUISMO_HPP
#define TRANSLATION_GUISMO_HPP
#include "odfaeg/Physics/boundingPolyhedron.h"
#include "odfaeg/Graphics/convexShape.h"
#include "odfaeg/Graphics/rectangleShape.h"
#include "odfaeg/Graphics/renderTarget.h"
class TranslationGuismo : public odfaeg::graphic::Drawable, public odfaeg::graphic::Transformable {
public :
    TranslationGuismo(float arrowSize);
    void setCenterSize(odfaeg::math::Vec3f center, odfaeg::math::Vec3f size);
    void draw(odfaeg::graphic::RenderTarget& target, odfaeg::graphic::RenderStates states);
    bool intersectsXArrow(odfaeg::math::Ray ray);
    bool intersectsYArrow(odfaeg::math::Ray ray);
    bool intersectsZArrow(odfaeg::math::Ray ray);
    void move(odfaeg::math::Vec3f t);
    void setVisible(bool visible);
private :
    odfaeg::graphic::ConvexShape xArrow, yArrow, zArrow;
    odfaeg::graphic::RectangleShape xRect, yRect, zRect;
    odfaeg::physic::BoundingPolyhedron bpXArrow, bpYArrow, bpZArrow;
    float arrowSize;
    bool visible;
};
#endif // TRANSLATION_GUISMO_HPP

#ifndef SCALE_GUISMO_HPP
#define SCALE_GUISMO_HPP
#include "odfaeg/Physics/boundingBox.h"
#include "odfaeg/Graphics/renderTarget.h"
#include "odfaeg/Graphics/rectangleShape.h"
class ScaleGuismo : public odfaeg::graphic::Drawable, public odfaeg::graphic::Transformable {
public :
    ScaleGuismo (float rectSize);
    void setCenterSize(odfaeg::math::Vec3f center, odfaeg::math::Vec3f size);
    void draw(odfaeg::graphic::RenderTarget& target, odfaeg::graphic::RenderStates states);
    bool intersectsXRect(odfaeg::math::Ray ray, odfaeg::math::Vec3f& i1);
    bool intersectsYRect(odfaeg::math::Ray ray, odfaeg::math::Vec3f& i1);
    bool intersectsZRect(odfaeg::math::Ray ray, odfaeg::math::Vec3f& i1);
    void scale(odfaeg::math::Vec3f s);
    void setVisible(bool visible);
private:
    bool visible;
    odfaeg::graphic::RectangleShape xRect, yRect, zRect, xScaleRect, yScaleRect, zScaleRect;
    float rectSize;
    odfaeg::physic::BoundingBox bbXScaleRect, bbYScaleRect, bbZScaleRect;
};
#endif // SCALE_GUISMO_HPP

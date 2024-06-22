#include "../../../include/odfaeg/Graphics/circleShape.h"
using namespace sf;
namespace odfaeg {
    namespace graphic {
            ////////////////////////////////////////////////////////////
        CircleShape::CircleShape(float radius, unsigned int pointCount) :
        m_radius    (radius),
        m_pointCount(pointCount)
        {
            update();
        }


        ////////////////////////////////////////////////////////////
        void CircleShape::setRadius(float radius)
        {
            m_radius = radius;
            update();
        }


        ////////////////////////////////////////////////////////////
        float CircleShape::getRadius() const
        {
            return m_radius;
        }


        ////////////////////////////////////////////////////////////
        void CircleShape::setPointCount(unsigned int count)
        {
            m_pointCount = count;
            update();
        }

        ////////////////////////////////////////////////////////////
        unsigned int CircleShape::getPointCount() const
        {
            return m_pointCount;
        }


        ////////////////////////////////////////////////////////////
        Vector3f CircleShape::getPoint(unsigned int index) const
        {
            static const float pi = 3.141592654f;

            float angle = index * 2 * pi / m_pointCount - pi / 2;
            float x = std::cos(angle) * m_radius;
            float y = std::sin(angle) * m_radius;

            return Vector3f(m_radius + x, m_radius + y, 0);
        }
    }

} // namespace sf


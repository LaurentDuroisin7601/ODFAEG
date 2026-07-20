module;
//import odfaeg.graphic.rectangleShape;
module odfaeg.graphic.rectangleShape;
namespace odfaeg {
	namespace graphic {
        ////////////////////////////////////////////////////////////
        RectangleShape::RectangleShape(Device& device, const math::Vec3f& size) : Shape(device)
        {
            setSize(size);
            m_size = size;
            update();
        }


        ////////////////////////////////////////////////////////////
        void RectangleShape::onResize(math::Vec3f& scale)
        {
            m_size = getSize();
            getTransform().setScale(math::Vec3f(1.f, 1.f, 1.f));
            update();
        }
        ////////////////////////////////////////////////////////////
        void RectangleShape::onScale(math::Vec3f& scale)
        {
            m_size = getSize();
            getTransform().setScale(math::Vec3f(1.f, 1.f, 1.f));
            update();
        }

        ////////////////////////////////////////////////////////////
        unsigned int RectangleShape::getPointCount() const
        {
            return 4;
        }


        ////////////////////////////////////////////////////////////
        math::Vec3f RectangleShape::getPoint(unsigned int index) const
        {
            switch (index)
            {
            default:
            case 0: return math::Vec3f(0.f, 0.f, m_size.z());
            case 1: return math::Vec3f(m_size.x(), 0.f, m_size.z());
            case 2: return math::Vec3f(m_size.x(), m_size.y(), m_size.z());
            case 3: return math::Vec3f(0.f, m_size.y(), m_size.z());
            }
        }
    }
}

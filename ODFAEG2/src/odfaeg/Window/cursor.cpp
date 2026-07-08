module;
#include <cstdint>
//import odfaeg.window.cursor;
module odfaeg.window.cursor;
import odfaeg.math.vec;
import odfaeg.window.iCursorType;
import odfaeg.window.cursorImpl;
namespace odfaeg {
    namespace window {
        ////////////////////////////////////////////////////////////
        Cursor::Cursor() :
            m_impl(new CursorImpl())
        {
            // That's it
        }


        ////////////////////////////////////////////////////////////
        Cursor::~Cursor()
        {
            delete m_impl;
        }


        ////////////////////////////////////////////////////////////
        bool Cursor::loadFromPixels(const std::uint8_t* pixels, math::Vector2u size, math::Vector2u hotspot)
        {
            return m_impl->loadFromPixels(pixels, size, hotspot);
        }


        ////////////////////////////////////////////////////////////
        bool Cursor::loadFromSystem(ICursorType type)
        {
            return m_impl->loadFromSystem(type);
        }


        ////////////////////////////////////////////////////////////
        const CursorImpl& Cursor::getImpl() const
        {
            return *m_impl;
        }
    }
}
module;
#include <cstdint>
#include <odfaeg/config.hpp>
import odfaeg.window.cursorImpl;
module odfaeg.window.cursorImpl;
import odfaeg.window.iCursorType;
import odfaeg.math.vec;
#if defined(ODFAEG_SYSTEM_WINDOWS)
import odfaeg.window.win32Cursor;
typedef odfaeg::window::Win32Cursor CursorImplType;
#endif
namespace odfaeg {
    namespace window {
        ////////////////////////////////////////////////////////////
		CursorImpl::CursorImpl() : CursorImplType()
        {
            // That's it
        }


        ////////////////////////////////////////////////////////////
        bool CursorImpl::loadFromPixels(const std::uint8_t* pixels, math::Vector2u size, math::Vector2u hotspot)
        {
            if ((pixels == 0) || (size.x() == 0) || (size.y() == 0))
                return false;
            else
                return CursorImplType::loadFromPixels(pixels, size, hotspot);
        }


        ////////////////////////////////////////////////////////////
        bool CursorImpl::loadFromSystem(ICursorType type)
        {
            return CursorImplType::loadFromSystem(type);
        }


        ////////////////////////////////////////////////////////////
        const CursorImplType& CursorImpl::getImplType() const
        {
            return *this;
        }
    }
}
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
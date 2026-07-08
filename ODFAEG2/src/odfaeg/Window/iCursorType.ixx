export module odfaeg.window.iCursorType;

export namespace odfaeg::window {
    ////////////////////////////////////////////////////////////
        /// \brief Enumeration of the native system cursor types
        ///
        /// Refer to the following table to determine which cursor
        /// is available on which platform.
        ///
        ///  Type                               | Linux | Mac OS X | Windows  |
        /// ------------------------------------|:-----:|:--------:|:--------:|
        ///  sf::Cursor::Arrow                  |  yes  |    yes   |   yes    |
        ///  sf::Cursor::ArrowWait              |  no   |    no    |   yes    |
        ///  sf::Cursor::Wait                   |  yes  |    no    |   yes    |
        ///  sf::Cursor::Text                   |  yes  |    yes   |   yes    |
        ///  sf::Cursor::Hand                   |  yes  |    yes   |   yes    |
        ///  sf::Cursor::SizeHorizontal         |  yes  |    yes   |   yes    |
        ///  sf::Cursor::SizeVertical           |  yes  |    yes   |   yes    |
        ///  sf::Cursor::SizeTopLeftBottomRight |  no   |    yes*  |   yes    |
        ///  sf::Cursor::SizeBottomLeftTopRight |  no   |    yes*  |   yes    |
        ///  sf::Cursor::SizeAll                |  yes  |    no    |   yes    |
        ///  sf::Cursor::Cross                  |  yes  |    yes   |   yes    |
        ///  sf::Cursor::Help                   |  yes  |    yes*  |   yes    |
        ///  sf::Cursor::NotAllowed             |  yes  |    yes   |   yes    |
        ///
        ///  * These cursor types are undocumented so may not
        ///    be available on all versions, but have been tested on 10.13
        ///
        ////////////////////////////////////////////////////////////
	enum class ICursorType
	{
		Arrow,                  ///< Arrow cursor (default)
		ArrowWait,              ///< Busy arrow cursor
		Wait,                   ///< Busy cursor
		Text,                   ///< I-beam, cursor when hovering over a field allowing text entry
		Hand,                   ///< Pointing hand cursor
		SizeHorizontal,         ///< Horizontal double arrow cursor
		SizeVertical,           ///< Vertical double arrow cursor
		SizeTopLeftBottomRight, ///< Double arrow cursor going from top-left to bottom-right
		SizeBottomLeftTopRight, ///< Double arrow cursor going from bottom-left to top-right
		SizeAll,                ///< Combination of SizeHorizontal and SizeVertical
		Cross,                  ///< Crosshair cursor
		Help,                   ///< Help cursor
		NotAllowed              ///< Action not allowed cursor
	};	
}
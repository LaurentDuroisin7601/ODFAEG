module; 
#include <windows.h>
#include <cstdint>
#include <iostream>
import odfaeg.window.win32Cursor;
module odfaeg.window.win32Cursor;
import odfaeg.window.iCursorType;

namespace odfaeg {
    namespace window {
        ////////////////////////////////////////////////////////////
        Win32Cursor::Win32Cursor() :
            m_cursor(NULL)
        {
            // That's it.
        }


        ////////////////////////////////////////////////////////////
        Win32Cursor::~Win32Cursor()
        {
            release();
        }


        ////////////////////////////////////////////////////////////
        bool Win32Cursor::loadFromPixels(const std::uint8_t* pixels, math::Vector2u size, math::Vector2u hotspot)
        {
            release();

            // Create the bitmap that will hold our color data
            BITMAPV5HEADER bitmapHeader;
            std::memset(&bitmapHeader, 0, sizeof(BITMAPV5HEADER));

            bitmapHeader.bV5Size = sizeof(BITMAPV5HEADER);
            bitmapHeader.bV5Width = size.x();
            bitmapHeader.bV5Height = -static_cast<int>(size.y()); // Negative indicates origin is in upper-left corner
            bitmapHeader.bV5Planes = 1;
            bitmapHeader.bV5BitCount = 32;
            bitmapHeader.bV5Compression = BI_BITFIELDS;
            bitmapHeader.bV5RedMask = 0x00ff0000;
            bitmapHeader.bV5GreenMask = 0x0000ff00;
            bitmapHeader.bV5BlueMask = 0x000000ff;
            bitmapHeader.bV5AlphaMask = 0xff000000;

            std::uint32_t* bitmapData = NULL;

            HDC screenDC = GetDC(NULL);
            HBITMAP color = CreateDIBSection(
                screenDC,
                reinterpret_cast<const BITMAPINFO*>(&bitmapHeader),
                DIB_RGB_COLORS,
                reinterpret_cast<void**>(&bitmapData),
                NULL,
                0
            );
            ReleaseDC(NULL, screenDC);

            if (!color)
            {
                std::cerr << "Failed to create cursor color bitmap" << std::endl;
                return false;
            }

            // Fill our bitmap with the cursor color data
            // We'll have to swap the red and blue channels here
            std::uint32_t* bitmapOffset = bitmapData;
            for (std::size_t remaining = size.x() * size.y(); remaining > 0; --remaining, pixels += 4)
            {
                *bitmapOffset++ = (pixels[3] << 24) | (pixels[0] << 16) | (pixels[1] << 8) | pixels[2];
            }

            // Create a dummy mask bitmap (it won't be used)
            HBITMAP mask = CreateBitmap(size.x(), size.y(), 1, 1, NULL);

            if (!mask)
            {
                DeleteObject(color);
                std::cerr << "Failed to create cursor mask bitmap" << std::endl;
                return false;
            }

            // Create the structure that describes our cursor
            ICONINFO cursorInfo;
            std::memset(&cursorInfo, 0, sizeof(ICONINFO));

            cursorInfo.fIcon = FALSE; // This is a cursor and not an icon
            cursorInfo.xHotspot = hotspot.x();
            cursorInfo.yHotspot = hotspot.y();
            cursorInfo.hbmColor = color;
            cursorInfo.hbmMask = mask;

            // Create the cursor
            m_cursor = reinterpret_cast<HCURSOR>(CreateIconIndirect(&cursorInfo));

            // The data has been copied into the cursor, so get rid of these
            DeleteObject(color);
            DeleteObject(mask);

            if (m_cursor)
            {
                return true;
            }
            else
            {
                std::cerr << "Failed to create cursor from bitmaps" << std::endl;
                return false;
            }
        }


        ////////////////////////////////////////////////////////////
        bool Win32Cursor::loadFromSystem(ICursorType type)
        {
            release();

            LPCTSTR shape;
            switch (type)
            {
            case ICursorType::Arrow:                  shape = IDC_ARROW;       break;
            case ICursorType::ArrowWait:              shape = IDC_APPSTARTING; break;
            case ICursorType::Wait:                   shape = IDC_WAIT;        break;
            case ICursorType::Text:                   shape = IDC_IBEAM;       break;
            case ICursorType::Hand:                   shape = IDC_HAND;        break;
            case ICursorType::SizeHorizontal:         shape = IDC_SIZEWE;      break;
            case ICursorType::SizeVertical:           shape = IDC_SIZENS;      break;
            case ICursorType::SizeTopLeftBottomRight: shape = IDC_SIZENWSE;    break;
            case ICursorType::SizeBottomLeftTopRight: shape = IDC_SIZENESW;    break;
            case ICursorType::SizeAll:                shape = IDC_SIZEALL;     break;
            case ICursorType::Cross:                  shape = IDC_CROSS;       break;
            case ICursorType::Help:                   shape = IDC_HELP;        break;
            case ICursorType::NotAllowed:             shape = IDC_NO;          break;
            }

            // Create a copy of the shared system cursor that we can destroy later
            m_cursor = CopyCursor(LoadCursor(NULL, shape));

            if (m_cursor)
            {
                return true;
            }
            else
            {
                std::cerr << "Could not create copy of a system cursor" << std::endl;
                return false;
            }
        }
        HCURSOR Win32Cursor::getCursor() const {
            return m_cursor;
        }
        ////////////////////////////////////////////////////////////
        void Win32Cursor::release()
        {
            if (m_cursor) {
                DestroyCursor(m_cursor);
                m_cursor = NULL;
            }
        }
    }
}
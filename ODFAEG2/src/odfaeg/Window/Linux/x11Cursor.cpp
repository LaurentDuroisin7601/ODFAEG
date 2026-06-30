module;
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <cstdint>
#include <iostream>
#include <vector>
import odfaeg.window.x11Cursor;
module odfaeg.window.x11Cursor;
import odfaeg.window.display;
import odfaeg.window.iCursorType;
namespace odfaeg {
    namespace window {
        ////////////////////////////////////////////////////////////
        X11Cursor::X11Cursor()
        {
            m_display = Display::openDisplay();
        }


        ////////////////////////////////////////////////////////////
        X11Cursor::~X11Cursor()
        {
            release();
            Display::closeDisplay(m_display);
        }


        ////////////////////////////////////////////////////////////
        bool X11Cursor::loadFromPixels(const std::uint8_t* pixels, math::Vector2u size, math::Vector2u hotspot)
        {
            release();

            // Convert the image into a bitmap (monochrome!).
            std::size_t bytes = (size.x() + 7) / 8 * size.y();
            std::vector<std::uint8_t> mask(bytes, 0); // Defines which pixel is transparent.
            std::vector<std::uint8_t> data(bytes, 1); // Defines which pixel is white/black.

            for (std::size_t j = 0; j < size.y(); ++j)
            {
                for (std::size_t i = 0; i < size.x(); ++i)
                {
                    std::size_t pixelIndex = i + j * size.x();
                    std::size_t byteIndex  = pixelIndex / 8;
                    std::size_t bitIndex   = i % 8;

                    // Turn on pixel that are not transparent
                    std::uint8_t opacity = pixels[pixelIndex * 4 + 3] > 0 ? 1 : 0;
                    mask[byteIndex] |= opacity << bitIndex;

                    // Choose between black/background & white/foreground color for each pixel,
                    // based on the pixel color intensity: on average, if a channel is "active"
                    // at 25%, the bit is white.
                    int intensity = pixels[pixelIndex * 4 + 0] + pixels[pixelIndex * 4 + 1] + pixels[pixelIndex * 4 + 2];
                    std::uint8_t bit = intensity > 64 ? 1 : 0;
                    data[byteIndex] |= bit << bitIndex;
                }
            }

            Pixmap maskPixmap = XCreateBitmapFromData(m_display, XDefaultRootWindow(m_display),
                                                      (char*)&mask[0], size.x(), size.y());
            Pixmap dataPixmap = XCreateBitmapFromData(m_display, XDefaultRootWindow(m_display),
                                                      (char*)&data[0], size.x(), size.y());

            // Define the foreground color as white and the background as black.
            XColor fg, bg;
            fg.red = fg.blue = fg.green = -1;
            bg.red = bg.blue = bg.green =  0;

            // Create the monochrome cursor.
            m_cursor = XCreatePixmapCursor(m_display,
                                           dataPixmap, maskPixmap,
                                           &fg, &bg,
                                           hotspot.x(), hotspot.y());

            // Free the resources
            XFreePixmap(m_display, dataPixmap);
            XFreePixmap(m_display, maskPixmap);

            // We assume everything went fine...
            return true;
        }


        ////////////////////////////////////////////////////////////
        bool X11Cursor::loadFromSystem(ICursorType type)
        {
            release();
            unsigned int shape;
            switch (type)
            {
                default: return false;

                case ICursorType::Arrow:          shape = XC_arrow;              break;
                case ICursorType::Wait:           shape = XC_watch;              break;
                case ICursorType::Text:           shape = XC_xterm;              break;
                case ICursorType::Hand:           shape = XC_hand1;              break;
                case ICursorType::SizeHorizontal: shape = XC_sb_h_double_arrow;  break;
                case ICursorType::SizeVertical:   shape = XC_sb_v_double_arrow;  break;
                case ICursorType::SizeAll:        shape = XC_fleur;              break;
                case ICursorType::Cross:          shape = XC_crosshair;          break;
                case ICursorType::Help:           shape = XC_question_arrow;     break;
                case ICursorType::NotAllowed:     shape = XC_X_cursor;           break;
            }

            m_cursor = XCreateFontCursor(m_display, shape);
            return true;
        }
        ::Cursor X11Cursor::getCursor() const {
            return m_cursor;
        }
        ////////////////////////////////////////////////////////////
        void X11Cursor::release()
        {
            XFreeCursor(m_display, m_cursor);
        }
    }
}//
// Created by laurent on 21/05/2026.
//

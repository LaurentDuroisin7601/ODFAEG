#ifndef ODFAEG_VIDEO_MODE
#define ODFAEG_VIDEO_MODE
#include <SDL2/SDL.h>
#include <vector>
namespace odfaeg {
class VideoMode {
public :
    VideoMode(unsigned int width, unsigned int height, unsigned int bitsPerPixels);
    static std::vector<VideoMode> getFullscreenModes();
    unsigned int getWidth();
    unsigned int getHeight();
    unsigned int getBitsPerPixel();
    static VideoMode getDesktopMode();
    bool isValid() const;
private :
    unsigned int width;
    unsigned int height;
    unsigned int bitsPerPixel;
    friend bool operator ==(const VideoMode& left, const VideoMode& right);

    friend bool operator !=(const VideoMode& left, const VideoMode& right);

    friend bool operator <(const VideoMode& left, const VideoMode& right);

    friend bool operator >(const VideoMode& left, const VideoMode& right);

    friend bool operator <=(const VideoMode& left, const VideoMode& right);

    friend bool operator >=(const VideoMode& left, const VideoMode& right);
};

bool operator ==(const VideoMode& left, const VideoMode& right);

bool operator !=(const VideoMode& left, const VideoMode& right);

bool operator <(const VideoMode& left, const VideoMode& right);

bool operator >(const VideoMode& left, const VideoMode& right);

bool operator <=(const VideoMode& left, const VideoMode& right);

bool operator >=(const VideoMode& left, const VideoMode& right);
}
#endif // ODFAEG_VIDEO_MODE

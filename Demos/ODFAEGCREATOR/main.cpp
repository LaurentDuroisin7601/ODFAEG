#include "application.hpp"
#include "../../ODFAEG/include/odfaeg/Core/multdispvisitors.hpp"
#include "../../ODFAEG/include/odfaeg/Core/clock.h"
using namespace odfaeg::graphic::gui;
using namespace odfaeg::graphic;
using namespace odfaeg::physic;
using namespace odfaeg::math;
using namespace odfaeg::window;

int main(int argc, char* argv[])
{
    ODFAEGCreator app(VideoMode(1000,700),"ODFAEG Creator");
    return app.exec();
}

#include "application.h"
#include <stdlib.h>     /* atexit */
#include "odfaeg/Network/cliEncryptedPacket.hpp"
using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::graphic;
using namespace odfaeg::physic;
using namespace odfaeg::network;
using namespace sorrok;
using namespace sf;
using namespace std;
int main (int argv, char* argc[]) {


    MyAppli app;
    return app.exec();
}


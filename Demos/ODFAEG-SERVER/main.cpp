#include "application.h"
#include <QtCore/QCoreApplication>
using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::graphic;
using namespace odfaeg::physic;
using namespace odfaeg::network;
using namespace sorrok;
using namespace sf;
using namespace std;
int main (int argv, char* argc[]) {

    QCoreApplication a(argv, argc);
    MyAppli app;
    return app.exec();
}


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
    EXPORT_CLASS_GUID(BoundingVolumeBoundingBox, BoundingVolume, BoundingBox)
    EXPORT_CLASS_GUID(EntityTile, Entity, Tile)
    EXPORT_CLASS_GUID(EntityTile, Entity, BigTile)
    EXPORT_CLASS_GUID(EntityWall, Entity, g2d::Wall)
    EXPORT_CLASS_GUID(EntityDecor, Entity, g2d::Decor)
    EXPORT_CLASS_GUID(EntityAnimation, Entity, Anim)
    EXPORT_CLASS_GUID(EntityHero, Entity, Hero)
    EXPORT_CLASS_GUID(EntityMonster, Entity, Monster)
    EXPORT_CLASS_GUID(EntityPnj, Entity, Pnj)
    EXPORT_CLASS_GUID(EntityMesh, Entity, Mesh)
    EXPORT_CLASS_GUID(EntityPLIGHT, Entity, g2d::PonctualLight)
    EXPORT_CLASS_GUID(EntityPSystem, Entity, ParticleSystem)

    MyAppli app;
    return app.exec();
}


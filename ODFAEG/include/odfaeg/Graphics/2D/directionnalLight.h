#ifndef ODFAEG_DIRECTIONNAL_LIGHT_2D_HPP
#define ODFAEG_DIRECTIONNAL_LIGHT_2D_HPP
#include "../entityLight.h"
#include "../vertexArray.h"
namespace odfaeg {
    namespace graphic {
        namespace g2d {
            class ODFAEG_GRAPHICS_API DirectionnalLight : public EntityLight {
            public :

                DirectionnalLight (math::Vec3f center, math::Vec3f dir, int height, float r1, float r2, float r3, float intensity, sf::Color color, int quality, EntityFactory& entity, Entity* parent = nullptr);
                int getLightId() {
                    return Entity::getId();
                }
                void initTriangles ();
                void addTriangle(VertexArray *triangle);
                /*void intersectsWallCompletely(Vec2f &pointI1, Vec2f &pointI2, int i1, int i2, int numTris);
                void intersectsWallCorner (Vec2f &pointI1, Vec2f &pointI2, int n, Segment &s1, int i1);
                void checkInterTri(int n, Segment &s1);*/
                math::Vec3f getDir();
                int getLightType ();
                int getIntensity();
                std::vector<VertexArray*> getTris ();
                std::vector<VertexArray*> getConeTris();
                int compAlphaFromPoint(math::Vec2f &point);
                void onDraw (RenderTarget &target, RenderStates states) const;
                bool operator== (Entity &other);
                virtual ~DirectionnalLight ();
            private :
                void computeDirLightSize(math::Vec3f &origCone);
                void computeCone ();
                math::Vec3f dir;
                float intensity, littleRadius, bigRadius;
                int quality;
                std::vector<VertexArray*> triangles;
                std::vector<VertexArray*> coneTris;
            };
        }
    }
}
#endif



#ifndef ODFAEG_PONCTUAL_LIGHT_HPP
#define ODFAEG_PONCTUAL_LIGHT_HPP
#include "../light.h"
#include "../../../../include/odfaeg/Math/ray.h"
#include "../entity.h"
#include "../entityLight.h"
#include "../vertexArray.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        namespace g2d {
            /**
            * \file ponctualLight.h
            * \class PonctualLight
            * \author Duroisin.L
            * \version 1.0
            * \date 1/02/2014
            * \brief represent a PonctualLight,
            */
            class ODFAEG_GRAPHICS_API PonctualLight : public EntityLight {
            public :
                /**
                * \fn PonctualLight()
                * \brief constructor.
                */
                PonctualLight(EntityFactory& factory) : EntityLight(math::Vec3f(0, 0, 0),sf::Color(0, 0, 0, 0),0, 0, 0, 0,"E_PONCTUAL_LIGHT", factory, "",nullptr) {
                }
                /**
                * \fn PonctualLight(math::Vec3f center, float radius1, float radius2, float radius3, float intensity, sf::Color color, int quality, Entity *parent = nullptr);
                * \brief constructor.
                * \param math::Vec3f center : the center of the light.
                * \param float radius1 : the x radius of the light.
                * \param float radius2 : the y radius of the light.
                * \param float radius3 : the z radius of the light.
                */
                PonctualLight(math::Vec3f center, float radius1, float radius2, float radius3, float intensity, sf::Color color, int quality, EntityFactory& factory, Entity *parent = nullptr);
                /**
                * \fn void initTriangles ();
                * \brief init the triangles of the light.
                */
                Entity* clone();
                void initTriangles ();
                /**
                * \fn int& getLightId()
                * \brief get the id of the light.
                * \return the id of the light.
                */
                int getLightId() {
                    return Entity::getId();
                }
                /**
                * \fn void addTriangle(VertexArray *triangle);
                * \brief add a  triangle to the light.
                * \param triangle : the triangle to add.
                */
                void addTriangle(VertexArray *triangle);
                /**
                * \fn void intersectsWallCompletely(math::Vec3f &pointI1, math::Vec3f &pointI2, int i1, int i2, int numTris);
                * \brief reset the vertices of a triangle of the light if the segment intersect the triangle completely.
                * \param pointI1 : the first point of the intersection.
                * \param pointI2 : the second point of the intersection.
                * \param i1 : the index of the first triangle's edge which intersects the segment.
                * \param i2 : the index of the second triangle's edge which intersects the segment.
                * \param numTri : the index of the triangle of the light which intersects the segment.
                */
                void intersectsWallCompletely(math::Vec3f &pointI1, math::Vec3f &pointI2, int i1, int i2, int numTris);
                /**
                * \fn  void intersectsWallCorner (math::Vec3f &pointI1, math::Vec3f &pointI2, int n, Ray &s1, int i1);
                * \brief reset the vertices of a triangle of the light if a segment's extremity is in the triangle.
                * \param pointI1 : the points of the intersection.
                * \param pointI2 : the extremity of the segment which is in the triangle.
                * \param int n : the number of the triangle of the light which intersects the segment.
                * \param s1 : the segment.
                * \param i1 : the index of the triangle's edge which intersects the segment.
                */
                void intersectsWallCorner (math::Vec3f &pointI1, math::Vec3f &pointI2, int n, math::Ray &s1, int i1);
                /**
                * \fn void checkInterTri(int n, Ray &s1);
                * \brief check if a segment intersects a triangle of the light, and reset the vertices of the triangle if it's the case.
                * \param the index of the triangle to check with.
                * \param s1 : the segment.
                */
                void checkInterTri(int n, math::Ray &s1);
                /**
                * \fn int getIntensity ();
                * \brief return the intensity of the light.
                * \return the intensity of the light.
                */
                int getIntensity ();
                /**
                * \fn std::vector<VertexArray*> getTris ();
                * \brief return the vertex array of the lights.
                * \return the vertex arrays of the light.
                */
                std::vector<VertexArray*> getTris ();
                /**
                * \fn int compAlphaFromPoint(math::Vec3f &point);
                * \brief compute the alpha component of a point of the light.
                * \param the point of the light.
                * \return the alpha value.
                */
                int compAlphaFromPoint(math::Vec3f &point);
                /**
                * \fn void onDraw (RenderTarget &target, RenderStates states) const;
                * \brief draw the light onto the render target.
                * \param target : the render target.
                * \param states : the states.
                */
                void onDraw (RenderTarget &target, RenderStates states);
                /**
                * \fn bool operator== (Entity &other);
                * \brief compare an entity with another.
                * \param other : the other entity.
                */
                bool operator== (Entity &other);
                /**
                * \fn void vtserialize(Archive & ar)
                * \brief serialize the light.
                * \param ar : the archive.
                */
                template <typename Archive>
                void vrserialize(Archive & ar) {
                    EntityLight::vtserialize(ar);
                    ar(littleRadius);
                    ar(bigRadius);
                    ar(intensity);
                    ar(quality);
                    ar(triangles);
                }
                float getRadius() {
                    return bigRadius;
                }
                /**
                 *\fn virtual ~PonctualLight ();
                 *\brief destructor.
                */
                virtual ~PonctualLight ();
            private :
                float littleRadius, bigRadius; /**>holds the little and the big radius of the light.*/
                float intensity; /** holds the intensity of the light.*/
                int quality; /** holds the quality of the light (the number of triangles used to render the light.)*/
                std::vector<VertexArray*> triangles; /**> holds the vertex arrays of the light.*/
            };
        }
    }
}
#endif // PONCTUAL_LIGHT

#ifndef ODFAEG_3D_PONCTUAL_LIGHT_HPP
#define ODFAEG_3D_PONCTUAL_LIGHT_HPP
#include "../light.h"
#include "../../../../include/odfaeg/Math/ray.h"
#include "../entity.h"
#include "../entityLight.h"
#include "../vertexArray.h"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
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
                PonctualLight(math::Vec3f center, float r1, float r2, float r3, float intensity, sf::Color color, int quality, EntityFactory& factory, Entity *parent=nullptr);

                Entity* clone();
                /**
                * \fn void initTriangles ();
                * \brief init the triangles of the light.
                */
                void initTriangles ();
                /**
                * \fn void addTriangle(VertexArray *triangle);
                * \brief add a  triangle to the light.
                * \param triangle : the triangle to add.
                */
                void addTriangle(VertexArray *triangle);
                /**
                * \fn int& getLightId()
                * \brief get the id of the light.
                * \return the id of the light.
                */
                int getLightId() {
                    return Entity::getId();
                }
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
#endif

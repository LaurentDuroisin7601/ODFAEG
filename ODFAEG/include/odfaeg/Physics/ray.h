#ifndef ODFAEG_RAY_HPP
#define ODFAEG_RAY_HPP
#include "../Math/vec4.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
using namespace math;
  /**
  * \file ray.h
  * \class Ray
  * \brief Manage a Ray.
  * \author Duroisin.L
  * \version 1.0
  * \date 1/02/2014
  *
  * Manage a ray and compute rays intersections.
  */
class Ray {
    private :
        Vec3f orig; /**< The origin of the ray.*/
        Vec3f ext; /**< The extremity of the ray. */
        Vec3f dir; /**< The direction of the ray. */
    public :
        /**
        * \fn Ray
        * \brief default constructor (build a ray of length 0 with an origin of (0, 0))
        */
        Ray();
        /**
        * \fn Ray (Vec3f orig, Vec3f ext)
        * \brief constructor (build a ray with the given origin and extremity)
        * \param orig : the origin of the ray.
        * \param ext : the etremity of the ray.
        */
        Ray(Vec3f orig, Vec3f ext);
        /**
        * \fn Vec2f getOrig()
        * \return the origin of the ray.
        */
        Vec3f& getOrig ();
        /**
        * \fn Vec2f getDir()
        * \return the direction of the ray.
        */
        Vec3f& getDir();
        /**
        * \fn Vec2f getExt()
        * \return the extremity of the ray.
        */
        Vec3f& getExt();
        /**
        * \fn float intersects (Segment &other)
        * \brief Test if two rays intersects. (return 1 if the two rays interssects, 0 or -1 if they don't intersects)
        * \return a value that determine if the two rays intersects.
        */
        float intersects(Ray &other);
        /**
        * \fn float intersectsWhere (Segment &other)
        * \brief Test if two rays intersects and return the distance of the intersection from the ray's origin.
        * \return the distance of the intersection from the ray's origin.
         (1 = the intersection is in the extremity of the ray, 0.5 = the intersection is in the middle of the ray.)
         return -1 if the rays don't intersect.
        * If you want to get the intersection's point you have to do this->origin + this->dir * the returned value.
        */
        float intersectsWhere (Ray &other);
        /**
        * \fn float intersectsWhereOther (Segment &other)
        * \brief Test if two rays intersects and return the distance of the intersection from the other ray's origin.
        * \return the distance of the intersection from the other ray's origin.
         (1 = the intersection is in the extremity of the other ray, 0.5 = the intersection is in the middle of the other ray.)
         return -1 if the rays don't intersect.
        * If you want to get the point of the intersection you have to do other.origin + other.dir * the returned value.
        */
        float intersectsWhereOther (Ray &other);
        /**
        * \fn void setOrig (Vec3f &orig)
        * \brief set the origin of the ray.
        * \param the origin of the ray.
        */
        void setOrig (Vec3f &orig);
        /**
        * \fn void setExt (Vec3f &ext)
        * \brief set the extremity of the ray.
        * \param the extremity of the ray.
        */
        void setExt (Vec3f &ext);
        /**
        * \fn void rotate (int angle, Vec3f &origin)
        * \brief rotate the ray around a point.
        * \param angle : the angle of the rotation.
        * \param Vec3f origin : the origin of the rotation.
        */
        void rotate (int angle, Vec3f origin);
        /**
        * \fn void scale (flaot sx, float sy, float sz, Vec3f &origin)
        * \brief scale the ray around a point.
        * \param sx : the x value of the scale.
        * \param sy : the y value of the scale.
        * \param sz : the z value of the scale.
        * \param Vec3f origin : the origin of the scale.
        */
        void scale (float sx, float sy, float sz, Vec3f origin);
        /**
        * \fn void translate (int dx, int dy, int dz, Vec2f &origin)
        * \brief move the ray.
        * \param dx : the x distance to move from
        * \param dy : the y distance to move from.
        * \param dz : the z distance to move from.
        * \param Vec2f origin : the origin of the translation.
        */
        void translate (int dx, int dy, int dz, Vec3f origin);
        template <typename Archive>
        void serialize(Archive & ar) {
            ar(orig);
            ar(ext);
            ar(dir);
        }
};
}
#endif

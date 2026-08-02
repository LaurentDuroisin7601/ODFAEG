#ifndef ODFAEG_RAY_HPP
#define ODFAEG_RAY_HPP
#include "vec.hpp"
#include "matrix.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace math {
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
        private:
            Vec3f orig; /**< The origin of the ray.*/
            Vec3f ext; /**< The extremity of the ray. */
            Vec3f dir; /**< The direction of the ray. */
        public:
            Ray();
            Ray(Vec3f orig, Vec3f ext);

            void setOrig(Vec3f& orig);
            void setExt(Vec3f& ext);
            Vec3f& getOrig();
            Vec3f& getExt();
            Vec3f& getDir();
            bool intersects(Ray& other);
            float intersectsWhere(Ray& other);
            float intersectsWhereOther(Ray& other);
            int whichSide(Vec3f point);
        };
    }
}
#include "ray.inl"
#endif
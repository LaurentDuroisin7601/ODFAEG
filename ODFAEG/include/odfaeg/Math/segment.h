#ifndef SEGMENT
#define SEGMENT
#include "vec2f.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace math {
          /**
          * \file segment.h
          * \class Segment
          * \brief Manage a Segment.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * Manage a segment and compute segments intersections.
          */
        class Segment {
            private :
                Vec2f orig; /**< The origin of the segment.*/
                Vec2f ext; /**< The extremity of the segment. */
                Vec2f dir; /**< The direction of the segment. */
            public :
                /**
                * \fn Segment
                * \brief default constructor (build a segment of length 0 with an origin of (0, 0))
                */
                Segment();
                /**
                * \fn Segment (Vec2f orig, Vec2f ext)
                * \brief constructor (build a segment with the given origin and extremity)
                * \param orig : the origin of the segment.
                * \param ext : the etremity of the segment.
                */
                Segment (Vec2f orig, Vec2f ext);
                /**
                * \fn Vec2f getOrig()
                * \return the origin of the segment.
                */
                Vec2f& getOrig ();
                /**
                * \fn Vec2f getDir()
                * \return the direction of the segment.
                */
                Vec2f& getDir();
                /**
                * \fn Vec2f getExt()
                * \return the extremity of the segment.
                */
                Vec2f& getExt();
                /**
                * \fn float intersects (Segment &other)
                * \brief Test if two segments intersects. (return 1 if the two segments interssects, 0 or -1 if they don't intersects)
                * \return a value that determine if the two segments intersects.
                */
                float intersects(Segment &other);
                /**
                * \fn float intersectsWhere (Segment &other)
                * \brief Test if two segments intersects and return the distance of the intersection from the segment's origin.
                * \return the distance of the intersection from the segment's origin.
                 (1 = the intersection is in the extremity of the segment, 0.5 = the intersection is in the middle of the segment.)
                 return -1 if the segments don't intersect.
                * If you want to get the intersection's point you have to do this->origin + this->dir * the returned value.
                */
                float intersectsWhere (Segment &other);
                /**
                * \fn float intersectsWhereOther (Segment &other)
                * \brief Test if two segments intersects and return the distance of the intersection from the other segment's origin.
                * \return the distance of the intersection from the other segment's origin.
                 (1 = the intersection is in the extremity of the other segment, 0.5 = the intersection is in the middle of the other segment.)
                 return -1 if the segments don't intersect.
                * If you want to get the point of the intersection you have to do other.origin + other.dir * the returned value.
                */
                float intersectWhereOther (Segment &other);
                /**
                * \fn void setOrig (Vec2f &orig)
                * \brief set the origin of the segment.
                * \param the origin of the segment.
                */
                void setOrig (Vec2f &orig);
                /**
                * \fn void setExt (Vec2f &ext)
                * \brief set the extremity of the segment.
                * \param the extremity of the segment.
                */
                void setExt (Vec2f &ext);
                /**
                * \fn void rotate (int angle, Vec2f &origin)
                * \brief rotate the segment around a point.
                * \param angle : the angle of the rotation.
                * \param Vec2f origin : the origin of the rotation.
                */
                void rotate (int angle, Vec2f origin);
                /**
                * \fn void scale (flaot sx, float sy, Vec2f &origin)
                * \brief scale the segment around a point.
                * \param sx : the x value of the scale.
                * \param sy : the y value of the scale.
                * \param Vec2f origin : the origin of the scale.
                */
                void scale (float sx, float sy, Vec2f origin);
                /**
                * \fn void translate (flaot dx, float dy, Vec2f &origin)
                * \brief move the segment.
                * \param dx : the x distance to move from
                * \param dy : the y distance to move from.
                * \param Vec2f origin : the origin of the translation.
                */
                void translate (int dx, int dy, Vec2f origin);
        };
    }
}
#endif

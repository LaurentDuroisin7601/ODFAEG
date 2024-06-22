#ifndef ODFAEG_BOUNDING_VOLUME_HPP
#define ODFAEG_BOUNDING_VOLUME_HPP
#include "../Math/vec4.h"
#include "../Math/ray.h"
#include <string>
#include <iostream>
#include "collisionResultSet.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace physic {
        /**
          * \file boundingVolume.h
          * \class BoudingVolume
          * \brief Manage a bounding volume for collision detection
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * Base class of all bouding volumes of the framework used for collision detection.
          *
          */
        class BoundingVolume;
        class BoundingBox;
        class OrientedBoundingBox;
        class BoundingSphere;
        class BoundingEllipsoid;
        class BoundingPolyhedron;
        class BaseInterface {
             public :
             virtual bool onIntersects(BaseInterface& other, CollisionResultSet::Info& infos) = 0;
             virtual bool onIntersects(BaseInterface& bi, math::Ray& ray, bool segment, CollisionResultSet::Info& infos) = 0;
             virtual bool onIntersects(BaseInterface& bi, math::Ray& ray, math::Vec3f& near, math::Vec3f& far, CollisionResultSet::Info& infos) = 0;
             virtual bool intersects (BoundingBox &bb, CollisionResultSet::Info& infos) = 0;
             virtual bool intersects (BoundingSphere &bs, CollisionResultSet::Info& infos) = 0;
             virtual bool intersects (BoundingEllipsoid &be, CollisionResultSet::Info& infos) = 0;
             virtual bool intersects (OrientedBoundingBox &ob, CollisionResultSet::Info& infos) = 0;
             virtual bool intersects (BoundingPolyhedron &bp, CollisionResultSet::Info& infos) = 0;
             virtual bool intersects (math::Ray& ray, bool segment, CollisionResultSet::Info& infos) = 0;
             virtual bool intersectsWhere (math::Ray& ray, math::Vec3f& near, math::Vec3f& far, CollisionResultSet::Info& infos) = 0;
             std::vector<BoundingVolume*> getChildren() {
                std::vector<BoundingVolume*> raws;
                for (unsigned int i = 0; i < children.size(); i++) {
                    raws.push_back(children[0].get());
                }
                return raws;
             }
             std::vector<std::unique_ptr<BoundingVolume>> children;
        };
        class BoundingVolume : public BaseInterface, public core::Registered<BoundingVolume> {
        public :
            BoundingVolume () {

            }
            void addChild(BoundingVolume* bv) {
                children.push_back(bv->clone());
            }
            bool intersects(BaseInterface& other, CollisionResultSet::Info& info) {
                std::vector<BoundingVolume*> tmpChildren = getChildren();
                std::vector<BoundingVolume*> tmpOtherChildren = other.getChildren();
                if (tmpChildren.size() == 0 && tmpOtherChildren.size() == 0) {
                    return onIntersects(other, info);
                }  else if (tmpChildren.size() == 0 && tmpOtherChildren.size() != 0) {

                      for (unsigned int i = 0; i < tmpOtherChildren.size(); i++) {
                          if (onIntersects(*tmpOtherChildren[i], info))
                                return true;
                      }
                } else {

                    for (unsigned int i = 0; i < tmpChildren.size(); i++) {
                        if (tmpOtherChildren.size() == 0) {
                            if (tmpChildren[i]->onIntersects(other, info))
                                    return true;

                        } else {
                            for (unsigned j = 0; j < tmpOtherChildren.size(); j++) {
                                 if (tmpChildren[i]->onIntersects(*tmpOtherChildren[j], info))
                                        return true;
                            }
                        }
                    }
                }
                return false;
            }
            bool intersects(math::Ray& ray, bool segment, CollisionResultSet::Info& info) {
                return onIntersects(*this, ray, segment, info);
            }
            bool intersectsWhere(math::Ray& ray, math::Vec3f& _near, math::Vec3f& _far, CollisionResultSet::Info& info) {
                return onIntersects(*this, ray, _near, _far, info);
            }
            virtual math::Vec3f getPosition() = 0;
            virtual math::Vec3f getSize() = 0;
            virtual math::Vec3f getCenter() = 0;
            virtual void move (math::Vec3f t) = 0;
            virtual std::unique_ptr<BoundingVolume> clone () = 0;
            template <typename Archive>
            void vtserialize(Archive & ar) {
                ar(children);
            }
            void setName (std::string name) {
                this->name = name;
            }
            std::string getName() {
                return name;
            }
            virtual bool operator==(BoundingVolume& other) {
                if (getChildren().size() != other.getChildren().size())
                    return false;
                for (unsigned int i = 0; i < getChildren().size(); i++) {
                    if (getChildren()[i] != other.getChildren()[i])
                        return false;
                }
                return getCenter() == other.getCenter() && getSize() == other.getSize();
            }
            virtual bool operator!=(BoundingVolume& other) {
                return !(*this == other);
            }
            virtual ~BoundingVolume() {}
            protected :
            BoundingVolume(const BoundingVolume& other) {

            }
            BoundingVolume& operator= (const BoundingVolume& other) {
                return *this;
            }
            private :
            std::string name;
        };
    }
}
#endif // BOUNDING_AREAS

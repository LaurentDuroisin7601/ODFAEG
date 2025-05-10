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
        class ODFAEG_PHYSICS_API BaseInterface {
             public :
             BaseInterface() {
                 parent = nullptr;
             }
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
             BoundingVolume* getParent() {
                 return parent;
             }
             std::vector<std::unique_ptr<BoundingVolume>> children;
             BoundingVolume* parent;
        };
        class ODFAEG_PHYSICS_API BoundingVolume : public BaseInterface, public core::Registered<BoundingVolume> {
        public :
            BoundingVolume () : BaseInterface() {

            }
            void addChild(BoundingVolume* bv) {
                std::unique_ptr<BoundingVolume> ptr;
                ptr.reset(bv);
                children.emplace_back(std::move(ptr));
                bv->parent = this;
            }

            bool intersects(BaseInterface& other, CollisionResultSet::Info& info) {
                if (getChildren().size() == 0 && other.getChildren().size() == 0) {
                    //Si on arrive à la fin de la hiérarchie des deux arbres et que il y a collision on renvoie vrai.
                    if(onIntersects(other, info)){
                        return true;
                    }
                }
                //Si le volume englobant 2 seulement à des enfants.
                if (getChildren().size() == 0 && other.getChildren().size() != 0) {
                      //Si les volumes englobants parents ne sont pas en collision alors les volumes englobants enfants non plus, il n'y a pas collision.
                      if (!onIntersects(other, info))
                        return false;
                      bool intersectsOne = false;
                      //Recherche si au moins un volume englobant enfant est en collision avec l'autre il y a collision.
                      for (unsigned int i = 0; i < other.getChildren().size(); i++) {
                         //Si le volume englobant est en collision avec l'enfant on réitère. (test plus précis)
                         if (onIntersects(*other.getChildren()[i], info)) {
                            bool inter = intersects(*other.getChildren()[i], info);
                            //Il faut faire attention de ne pas remettre intersectsOne à false si un volume englobant enfant suivant  le volume englobant enfant en collision n'est pas en collision.
                            if (!intersectsOne && inter)
                                intersectsOne = true;
                         }
                      }
                      return intersectsOne;
                }
                //Si le volument englobant 1 seulement à des enfants.
                if (getChildren().size() != 0 && other.getChildren().size() == 0) {
                    //Si les volumes englobants parents ne sont pas en collision alors les volumes englobants enfants non plus, il n'y a pas collision.
                    if (!onIntersects(other, info))
                        return false;
                    bool intersectsOne = false;
                    //Si au moins un volume englobant enfant est en collision avec l'autre il y a collision sinon non.
                    for (unsigned int i = 0; i < getChildren().size(); i++) {
                        //Si le volume englobant est en collision avec l'enfant on réitère. (test plus précis)
                        if (getChildren()[i]->onIntersects(other, info)) {
                            bool inter = getChildren()[i]->intersects(other, info);
                            //Il faut faire attention de ne pas remettre intersectsOne à false si un volume englobant enfant suivant  le volume englobant enfant en collision n'est pas en collision.
                            if (!intersectsOne && inter)
                                intersectsOne = true;
                        }
                    }
                    return intersectsOne;
                }
                //Si les deux volumes de collisions ont des enfants.
                if (getChildren().size() != 0 && other.getChildren().size() != 0) {
                    //Si les volumes englobants parents ne sont pas en collision alors les volumes englobants enfants non plus, il n'y a pas collision.
                    if (!onIntersects(other, info))
                        return false;
                    bool intersectsOne = false;
                    //Si au moins un volume englobant enfant est en collision avec un autre volume englobant enfant il y a collision sinon non.
                    for (unsigned int i = 0; i < getChildren().size(); i++) {
                        for (unsigned int j = 0; j < other.getChildren().size(); j++) {
                            //Si le volume englobant enfant est en collision avec l'enfant de l'autre on réitère. (test plus précis)
                            if (getChildren()[i]->onIntersects(*other.getChildren()[j], info)) {
                                bool inter = getChildren()[i]->intersects(*other.getChildren()[j], info);
                                //Il faut faire attention de ne pas remettre intersectsOne à false si un volume englobant enfant suivant  le volume englobant enfant en collision n'est pas en collision.
                                if (!intersectsOne && inter)
                                    intersectsOne = true;
                            }
                        }
                    }
                    return intersectsOne;
                }
                //Si il n'y a pas d'enfants et que les volumes ne sont pas en collision on retourne faux.
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
            virtual void scale(math::Vec3f s) = 0;
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
                name = other.name;
            }
            BoundingVolume& operator=(const BoundingVolume& other) {
                name = other.name;
                return *this;
            }
            private :
            std::string name;
        };
    }
}
#endif // BOUNDING_AREAS

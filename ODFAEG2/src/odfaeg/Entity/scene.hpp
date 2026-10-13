#ifndef ODFAEG_SCENE_HPP
#define ODFAEG_SCENE_HPP
#include "gameObject.hpp"
#include "tile.hpp"
#include "wall.hpp"
#include "scene.hpp"
#include "grid.hpp"
#include "tileSet.hpp"
#include "../Math/baseChangmementMatrix.hpp"
#include <iostream>
namespace odfaeg {
    namespace entity {
        class Scene {
        public:
            Scene (std::string name, int cellWidth, int cellHeight, int cellDepth);
            void generate_labyrinthe (std::vector<Tile*> tGround, std::vector<Wall*> walls, unsigned int squareSize, physic::BoundingBox &rect);
            void generate_terrain(std::vector<Tile*> tGround, unsigned int squareSize, physic::BoundingBox &rect);
            void generate_rectangular_arena(std::vector<Wall*> walls, unsigned int squareSize, physic::BoundingBox &rect);
            void setName (std::string name);
            string getName();
            int getCompImage(std::string resource);
            bool addEntity(GameObject *entity);
            bool removeEntity (GameObject *entity);
            bool deleteEntity (GameObject *entity);
            void rotateEntity(GameObject *entity, float angle, math::Vec3f axis);
            void scaleEntity(GameObject *entity, float sx, float sy, float sz);
            void moveEntity(GameObject *entity, float dx, float dy, float dz);
            void setBaseChangementMatrix (math::BaseChangementMatrix bm);
            Entity* getEntity(std::string name);
            std::vector<GameObject*> getEntities(string expression);
            std::vector<GameObject*> getEntitiesInBox (physic::BoundingBox bx, std::string type);
            bool collide(Entity *entity, math::Vec3f position);
            bool collide (Entity *entity);
            bool collide (Entity* entity, math::Ray ray);
            math::Vec3f physicallyBasedComputePos (Entity* entity, math::Vec3f oldCenter, math::Vec3f size, math::Vec3f velocity, float climbCapacity);
                       
            private :
            void removeComptImg (std::string resource); 
            void increaseComptImg(std::string resource);
            void decreaseComptImg (std::string resource);
            std::map<std::string, int> compResources;  
            GridMap<GameObject*> gridMap;
            std::vector<std::unique_ptr<GameObject>> entities;
            math::BaseChangementMatrix baseChangmentMatrix;
            std::string name;                                   
        };        
    }
}
#endif 
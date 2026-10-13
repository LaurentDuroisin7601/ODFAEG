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
        struct RuleBase {  
            virtual void addGameObject(GameObject& gameObject) = 0;          
            virtual void execute() = 0;
            virtual ~RuleBase() = default;
        };
        template <typename Behaviour, typename... ActorsLists>
        struct Rule : RuleBase { 
            unsigned int behaviourID;        
            std::vector<unsigned int> concernedFamiliesKeys;     
            std::vector<unsigned int> concernedActorsKeys;            
            std::tuple<std::vector<ActorsLists>...> actors;
            core::FastDelegate<void> ruleExecutor;
            Rule(core::FastDelegate<void> ruleExecutor, std::tuple<std::vector<ActorsLists>...> actors) : ruleExecutor(ruleExecutor), actors(actors) {
                
            }
            void addGameObject(GameObject& gameObject) {
                for (unsigned int i = 0; i < gameObject.behaviourIDs.size(); i++) {
                    if (gameObject.behaviourIDs[i] == behaviourID) {
                        integrate<>(gameObject);
                    }
                }
            }
            void execute() {
                iterate<>(std::make_index_sequence<sizeof...(ActorsLists)>());
            }
            template <std::size_t I = 0>
            void integrate(GameObject& gameObject) {                    
                if constexpr (I < sizeof...(ActorsLists)) {
                    if (concernedActorsKeys[I] == gameObject.typeKey 
                        && std::tuple_element_t<I, std::tuple<ActorsLists...>>::TagPos == gameObject.tagPos
                        && concernedFamiliesKeys[I] == gameObject.familyID) {
                        std::get<I>(actors).push_back(static_cast<std::tuple_element_t<I, std::tuple<ActorsLists...>>::VALUE_TYPE&>(gameObject));
                        std::get<I>(actors).back().typeKey = gameObject.typeKey;
                    } else {
                        integrate<I+1>(gameObject);
                    }
                }
            }
            template <std::size_t I = 0, size_t... Ints>
            void iterate(std::index_sequence<Ints...> seq)
            {
                if constexpr (I == sizeof...(ActorsLists))
                {
                    ruleExecutor();
                }
                else
                {
                    auto& list = std::get<I>(actors);

                    for (std::size_t j = 0; j < list.size(); j++)
                    {                        
                        ruleExecutor.setParam<I+1, std::tuple_element_t<I,  std::tuple<ActorsLists...>>, Behaviour, core::ph<Ints, ActorsLists>...>(list[j]);                        
                        iterate<I + 1>(seq);                        
                    }
                }
            }
        };   
        struct GameMode {
            std::map<std::string, RuleBase*> rules;
            void addRule(std::string name, RuleBase* rule) {
                rules.insert(std::make_pair(name, rule));
            }
            void applyRule(std::string name) {
                std::map<std::string, RuleBase*>::iterator it = rules.find(name);
                if (it != rules.end()) {
                    it->second->execute();
                }    
            }
        };         
        template <typename Func, typename... Vs, size_t... TypeIDs, size_t... I>
        requires std::is_invocable_v<
                Func,
                decltype(std::declval<Vs>().template get<TypeIDs>())...
            >
        core::FastDelegate<void> make_rule_impl_for(Func func, std::index_sequence<I...> seq, std::index_sequence<TypeIDs...> seq2) {
                                
            core::FastDelegate<void> visitor(core::DispatchVisitorFor<Func, Vs...>::template visit<TypeIDs...>, func, core::ph<I, Vs>()...);
            return visitor;
        }
        template <typename Func, typename... Vs, size_t... I, size_t... TypeIDs>
        requires (!std::is_invocable_v<
                Func,
                decltype(std::declval<Vs>().template get<TypeIDs>())...
            >)
        core::FastDelegate<void> make_rule_impl_for(Func func, std::index_sequence<I...> seq, std::index_sequence<TypeIDs...> seq2) {
            core::FastDelegate<void> visitor(core::DispatchVisitorFor<Func, Vs...>::template visit<TypeIDs...>, func, core::ph<I, Vs>()...);               
            
            return visitor;
        }   
        template <typename R, std::size_t... TypeIDs> 
        void initKeys(R rule, std::index_sequence<TypeIDs...> seq) {
        
            (rule->concernedActorsKeys.push_back(TypeIDs),...);
        }  
        template <size_t BHID, typename Func, size_t... TypeIDs, typename... Vs>        
        RuleBase* make_rule_for(Func func, std::vector<Vs>... vs) {
            core::FastDelegate<void> visitor = make_rule_impl_for<Func, Vs...>(func, std::make_index_sequence<sizeof...(Vs)>(), std::index_sequence<TypeIDs...>());
            
            Rule<Func, Vs...>* rule = new Rule<Func, Vs...> (visitor, std::make_tuple(vs...));
            //std::cout<<"create rule"<<std::endl;            
            initKeys(rule, std::index_sequence<TypeIDs...>());
            (rule->concernedFamiliesKeys.push_back(Vs::FamilyID),...);
            rule->behaviourID = BHID;
            return rule;
        }  
        template<size_t BHID,
        typename Func,
        size_t... IDs,
        typename... Vs>
        RuleBase*
        make_rule_from_sequence(
            Func func,
            std::index_sequence<IDs...>,
            std::vector<Vs>... vs)
        {
            return make_rule_for<BHID,
                Func,
                IDs...
            >(func, vs...);
        }      
        template<size_t BHID, typename Func, typename... Seqs, typename... Vs>
        std::vector<RuleBase*>
        make_rules_for(Func func,
                            std::vector<Vs>... vs)
        {
            return {
                make_rule_from_sequence<BHID>(
                    func,
                    Seqs{},
                    vs...
                )...
            };
        }     
    }
}
#endif 
 #ifndef ODFAEG_GAMEPLAY_HPP
 #define ODFAEG_GAMEPLAY_HPP
 #include "gameObject.hpp"
 #include "../Core/delegate.hpp"
namespace odfaeg {
    namespace entity { 
        template<class T>
        struct is_component
            : std::true_type
        {
        };
        /**
        *  \file  fastDelegate.h
        *  \class is_placeholder
        *  \brief trait class specialization if the type is a placeholder the compiler choose this class's version.
        *  \param T the type.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<size_t I, size_t J, class V, class... T>
        struct is_component<core::TypeList<I, J, V, T...>>
            : std::false_type
        {
        };
        template<typename X> inline constexpr bool is_component_v = is_component<X>::value;
        struct RuleBase {  
            virtual void addGameObject(GameObject& gameObject) = 0;          
            virtual void execute() = 0;
            virtual ~RuleBase() = default;
        };
        template <typename Behaviour, typename... ActorsLists>
        struct Rule : RuleBase { 
            unsigned int behaviourID; 
            unsigned int ruleID;       
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
                iterate<>(nullptr, std::make_index_sequence<sizeof...(ActorsLists)>());
            }
            template <std::size_t I = 0>
            void integrate(GameObject& gameObject) {                    
                if constexpr (I < sizeof...(ActorsLists)) {
                    if constexpr(!is_component_v<std::tuple_element_t<I,  std::tuple<ActorsLists...>>>) {
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
            }
            template <std::size_t I = 0, size_t... Ints>
            void iterate(GameObject* currentGameObject, std::index_sequence<Ints...> seq)
            {
                
                if constexpr (I == sizeof...(ActorsLists))
                {
                    ruleExecutor();
                } else if constexpr (is_component_v<std::tuple_element_t<I,  std::tuple<ActorsLists...>>>) {
                    std::tuple_element_t<I,  std::tuple<ActorsLists...>> component = EnttEntity::getComponent<std::tuple_element_t<I,  std::tuple<ActorsLists...>>>(*currentGameObject);
                    if (component.has_value()) {
                        ruleExecutor.setParam<I+1, std::tuple_element_t<I,  std::tuple<ActorsLists...>>, Behaviour, core::ph<Ints, ActorsLists>...>(component.value());     
                    }   
                    iterate<I + 1>(currentGameObject, seq);           
                }
                else
                {
                    auto& list = std::get<I>(actors);

                    for (std::size_t j = 0; j < list.size(); j++)
                    {                        
                        ruleExecutor.setParam<I+1, std::tuple_element_t<I,  std::tuple<ActorsLists...>>, Behaviour, core::ph<Ints, ActorsLists>...>(list[j]);                        
                        iterate<I + 1>(&list[j].get(), seq);                        
                    }
                }
            }
        };   
        struct GameMode {
            std::map<unsigned int, RuleBase*> rules;
            void addRule(unsigned int ruleId, RuleBase* rule) {
                rules.insert(std::make_pair(ruleId, rule));
            }
            void addRules(std::vector<RuleBase*> rules) {
                for (unsigned int i = 0; i < rules.size(); i++) {
                    addRule(rules[i]->ruleId, rules[i]);
                }
            }
            void applyRule(unsigned int ruleId) {
                std::map<unsigned int, RuleBase*>::iterator it = rules.find(ruleId);
                if (it != rules.end()) {
                    it->second->execute();
                }    
            }
            void applyAllRules() {
                std::map<unsigned int, RuleBase*>::iterator it;
                for (it = rules.begin(); it != rules.end(); it++) {
                    it->second->execute();
                }
            }
        };         
        
        template <typename R, std::size_t... TypeIDs> 
        void initKeys(R rule, std::index_sequence<TypeIDs...> seq) {
        
            (rule->concernedActorsKeys.push_back(TypeIDs),...);
        }  
        template <size_t BHID, typename Func, size_t... TypeIDs, typename... Vs>        
        RuleBase* make_rule(Func func, std::vector<Vs>... vs) {
            core::FastDelegate<void> visitor = core::make_visitor<Func, Vs...>(func, std::make_index_sequence<sizeof...(Vs)>(), std::index_sequence<TypeIDs...>());
            
            Rule<Func, Vs...>* rule = new Rule<Func, Vs...> (visitor, std::make_tuple(vs...));
            rule->ruleID = hash<TypeIDs...>();
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
            return make_rule<BHID,
                Func,
                IDs...
            >(func, vs...);
        }      
        template<size_t BHID, typename Func, typename... Seqs, typename... Vs>
        std::vector<RuleBase*>
        make_rules_impl(Func func,
                            std::vector<Vs>... vs, typeList<Seqs...> tl);
        {
            return {
                make_rule_from_sequence<BHID>(
                    func,
                    Seqs{},
                    vs...
                )...
            };            
        }
        template<size_t BHID, typename Func, typename... Vs>
        std::vector<RuleBase*>
        make_rules(Func func,
                            std::vector<Vs>... vs);
        {
            using Rules = core::cartesian_product<std::make_index_sequence<Vs::N>...>::type;  
            return make_rule_impl<BHID>(
                func,                    
                vs...,
                Rules()
            );                      
        }
    }
}
#endif     
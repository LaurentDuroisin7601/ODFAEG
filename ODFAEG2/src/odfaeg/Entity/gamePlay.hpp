 #ifndef ODFAEG_GAMEPLAY_HPP
 #define ODFAEG_GAMEPLAY_HPP
namespace odfaeg {
    namespace entity { 
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
        
        template <typename R, std::size_t... TypeIDs> 
        void initKeys(R rule, std::index_sequence<TypeIDs...> seq) {
        
            (rule->concernedActorsKeys.push_back(TypeIDs),...);
        }  
        template <size_t BHID, typename Func, size_t... TypeIDs, typename... Vs>        
        RuleBase* make_rule(Func func, std::vector<Vs>... vs) {
            core::FastDelegate<void> visitor = core::make_visitor<Func, Vs...>(func, std::make_index_sequence<sizeof...(Vs)>(), std::index_sequence<TypeIDs...>());
            
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
        make_rules(Func func,
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
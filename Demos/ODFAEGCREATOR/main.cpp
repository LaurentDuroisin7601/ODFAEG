#include "application.hpp"
#include "../../ODFAEG/include/odfaeg/Core/multdispvisitors.hpp"
#include "../../ODFAEG/include/odfaeg/Core/clock.h"
using namespace odfaeg::graphic::gui;
using namespace odfaeg::graphic;
using namespace odfaeg::physic;
using namespace odfaeg::math;
using namespace odfaeg::window;
//using namespace odfaeg::core;


        /*struct Command {
            FastDelegate<void> slot;
            std::unique_ptr<FastDelegate<bool>> trigger;

            std::string name;
            void setName(std::string name) {
                this->name = name;
            }
            Command() {
                this->trigger = nullptr;
            }
            Command (FastDelegate<bool> trigger, FastDelegate<void> slot) : slot(slot) {
                this->trigger = std::make_unique<FastDelegate<bool>>(trigger);
            }
            Command(const Command& other) : slot(other.slot) {
               if (other.trigger != nullptr) {
                   //std::cout<<"fd : "<<fd()<<std::endl;
                   trigger = std::make_unique<FastDelegate<bool>>(*other.trigger);
                    std::cout<<"command copy : "<<(*trigger)()<<std::endl;
               }
               name = other.name;
            }

            bool isTriggered()
            {

                if (trigger != nullptr) {
                    return (*trigger)();
                }
                return false;
            }
            void operator()()
            {
                slot();
            }

            Command& operator=(const Command& other) {
                if (other.trigger != nullptr) {
                    trigger = std::make_unique<FastDelegate<bool>>(*other.trigger);
                    std::cout<<"command copy affector : "<<(*trigger)()<<std::endl;
                }
                slot = FastDelegate<void>(other.slot);
                return *this;
            }
        };
        class Listener {

        public :

             Listener() {
             }

             void connect(std::string key, Command command) {
                std::cout<<"add command"<<std::endl;
                command.setName(key);
                commands[key] = command;
                //commands = new Command[1]{command};
             }


             void processEvents() {
                 std::map<std::string, Command>::iterator it;

                 for (it = commands.begin(); it != commands.end(); it++) {
                    std::cout<<"triggered ? "<<std::endl;
                    if (it->second.isTriggered()) {

                        (it->second)();
                    }
                 }
                 //std::cout<<commands[0].isTriggered()<<std::endl;
             }
             private :
             std::map<std::string, Command> commands;
             //Command* commands;
        };
        struct Test {
            Listener listener;
            RenderComponentManager rcm;
            Label* lab;
            Test(RenderWindow &rw) : rcm(rw) {
                Font font;
                font.loadFromFile("fonts/Arial.ttf");
                lab = new Label(rw, Vec3f(0, 0, 0),Vec3f(100, 50, 0),&font,"test",15);
                std::cout<<"lab : "<<lab<<std::endl;
                FastDelegate<bool> signal(&Label::isMouseInside, lab);
                FastDelegate<void> slot(&Test::onMouseInLab, this, lab);
                Command cmd(signal, slot);
                listener.connect("test", cmd);
                rcm.addComponent(lab);
                //std::cout<<cmd.isTriggered()<<std::endl;
            }
            void onMouseInLab(Label* lab) {
            }
            void processEvents() {
                listener.processEvents();
            }
        };*/




template < typename Tp, typename... List >
struct contains : std::true_type {};

template < typename Tp, typename Head, typename... Rest >
struct contains<Tp, Head, Rest...>
: std::conditional< std::is_same<Tp, Head>::value,
    std::true_type,
    contains<Tp, Rest...>
>::type {};

template < typename Tp >
struct contains<Tp> : std::false_type {};


template <typename T, typename U=void, typename... Types>
constexpr size_t index() {
    return std::is_same<T, U>::value ? 0 : 1 + index<T, Types...>();
}

class entity;
struct IEntityManager {
    unsigned int type;
    unsigned int containerIdx;
    std::string name;

};
template <class B, size_t I, size_t NB, class... Derived>
struct EntityManagerHolder {

};
template <class B, size_t I, size_t NB, class Head, class... Tail>
struct EntityManagerHolder<B, I, NB, Head, Tail...> : EntityManagerHolder<B, I+1, NB, Tail...>  {
    std::vector<B*> entities;

    void add(B* entity, size_t N) {
        if (I == N) {
            entity->type = I;
            entity->containerIdx = entities.size();
            entities.push_back(entity);
        } else {
            EntityManagerHolder<B, I+1, NB, Tail...>::add(entity, N);
        }
    }
    template <class H, class... T>
    void copy(EntityManagerHolder<B, I, NB-1, H, T...>& holder) {
        entities = holder.entities;
        EntityManagerHolder<B, I+1, NB,  Tail...>::copy(holder);
    }

    template <class R, typename EntityManagerArray>
    void apply (EntityManagerArray& ema, const size_t& typeIdx, const size_t& containerIdx, float x, float y, float z, R& ret) {
        if (I == typeIdx) {
            Head* entity = static_cast<Head*>(entities[containerIdx]);
            entity->move(ema, entity, x, y, z);
        } else {
            EntityManagerHolder<B, I+1, NB, Tail...>::apply(ema, typeIdx, containerIdx, x, y, z, ret);
        }
    }
    template <class EntityManagerArray>
    void applyAll(EntityManagerArray& ema, float x, float y, float z) {
        for (unsigned int i = 0; i < entities.size(); i++) {
            entities[i]->move(ema, static_cast<Head*>(entities[i]), x, y, z);
        }
        EntityManagerHolder<B, I+1, NB, Tail...>::applyAll(ema, x, y, z);
    }
};
template <class B, size_t I, size_t NB, class Head>
struct EntityManagerHolder<B, I, NB, Head> : EntityManagerHolder<B, I+1, NB>  {
    std::vector<B*> entities;
    void add(B* entity, size_t N) {
        if (I == N) {
            entity->type = I;
            entity->containerIdx = entities.size();
            entities.push_back(entity);
        }
    }
    template <class... T>
    void copy(EntityManagerHolder<B, I, NB-1>& holder) {

    }
    template <class R, typename EntityManagerArray>
    void apply (EntityManagerArray& ema, const size_t& typeIdx, const size_t& containerIdx, float x, float y, float z, R& ret) {
        if (I == typeIdx) {
            Head* entity = static_cast<Head*>(entities[containerIdx]);
            entity->move(ema, entity, x, y, z);
        }
    }
    template <class EntityManagerArray>
    void applyAll(EntityManagerArray& ema, float x, float y, float z) {
        for (unsigned int i = 0; i < entities.size(); i++) {
            static_cast<Head*>(entities[i])->move(ema, static_cast<Head*>(entities[i]), x, y, z);
        }
    }
};

template <class B, size_t I, size_t NB>
struct EntityManagerHolder<B, I, NB> {

    void add(B* entity) {

    }
    template <class... T>
    void copy(EntityManagerHolder<B, I, NB>& holder) {

    }
    template <class R, typename EntityManagerArray>
    void apply (EntityManagerArray& ema, const size_t& typeIdx, const size_t& containerIdx, float x, float y, float z, R& ret) {
    }
    void applyAll(float x, float y, float z) {
    }
};
template <typename Interface, typename B, typename... TupleTypes>
struct EntityManager : Interface {
    EntityManagerHolder<B, 0, sizeof...(TupleTypes), TupleTypes...> contents;
    template <typename H, class = typename std::enable_if_t<contains<H, TupleTypes...>::value>>
    EntityManager add (H* head) {
        head->type = index<H, TupleTypes...>();
        contents.add(head, head->type);
        return *this;
    }
    template <typename H, class = typename std::enable_if_t<!contains<H, TupleTypes...>::value>>
    EntityManager<Interface, B, TupleTypes..., H> add (H* head) {
        EntityManager<Interface, B, TupleTypes..., H> entityManager;
        entityManager.contents.template copy<TupleTypes...>(contents);
        return entityManager.add(head);
    }
    template <class EntityManagerArray>
    void applyAll(EntityManagerArray& ema, float x, float y, float z) {
        contents.applyAll(ema, x, y, z);
    }
    template <class R, typename EntityManagerArray>
    void apply (EntityManagerArray& ema, const size_t& typeIdx, const size_t& containerIdx, float x, float y, float z, R& ret) {
        contents.apply(ema, typeIdx, containerIdx, x, y, z, ret);
    }
};
template <class B, size_t I, size_t NB, class... Derived>
struct EntityManagerArrayHolder {

};
template <class B, size_t I, size_t NB, class Head, class... Tail>
struct EntityManagerArrayHolder<B, I, NB, Head, Tail...> : EntityManagerArrayHolder<B, I+1, NB, Tail...>  {
    std::vector<B*> entities;

    void add(B* entity, size_t N) {
        if (I == N) {
            entity->type = I;
            entity->containerIdx = entities.size();
            entities.push_back(entity);
        } else {
            EntityManagerArrayHolder<B, I+1, NB, Tail...>::add(entity, N);
        }
    }
    template <class H, class... T>
    void copy(EntityManagerArrayHolder<B, I, NB-1, H, T...>& holder) {
        entities = holder.entities;
        EntityManagerArrayHolder<B, I+1, NB,  Tail...>::copy(holder);
    }
    template <typename EntityManagerArray>
    void applyAll (EntityManagerArray& ema, const size_t& typeIdx, const size_t& containerIdx, float x, float y, float z) {
        if (typeIdx == I) {
            static_cast<Head*>(entities[containerIdx])->applyAll(ema, x, y, z);
        } else {
            EntityManagerArrayHolder<B, I+1, NB,  Tail...>::applyAll(ema, typeIdx, containerIdx, x, y, z);
        }
    }
};
template <class B, size_t I, size_t NB, class Head>
struct EntityManagerArrayHolder<B, I, NB, Head> : EntityManagerArrayHolder<B, I+1, NB>  {
    std::vector<B*> entities;
    void add(B* entity, size_t N) {
        if (I == N) {
            entity->type = I;
            entity->containerIdx = entities.size();
            entities.push_back(entity);
        }
    }
    static constexpr size_t nbTypes() {
        return 1;
    }
    template <class... T>
    void copy(EntityManagerArrayHolder<B, I, NB-1>& holder) {

    }
    template <typename EntityManagerArray>
    void applyAll (EntityManagerArray& ema, const size_t& typeIdx, const size_t& containerIdx, float x, float y, float z) {
        if (typeIdx == I) {
             static_cast<Head*>(entities[containerIdx])->applyAll(ema, x, y, z);
        }
    }
};

template <class B, size_t I, size_t NB>
struct EntityManagerArrayHolder<B, I, NB>  {
    std::vector<B*> entities;
    void add(B* entity) {

    }
    template <class... T>
    void copy(EntityManagerArrayHolder<B, I, NB>& holder) {

    }
    void applyAll (const size_t& typeIdx, const size_t& containerIdx, float x, float y, float z) {
    }
};
template <typename B, typename... TupleTypes>
struct EntityManagerArray {
    EntityManagerArrayHolder<B, 0, sizeof...(TupleTypes), TupleTypes...> contents;
    template <typename H, class = typename std::enable_if_t<contains<H, TupleTypes...>::value>>
    EntityManagerArray add (H* head) {
        head->type = index<H, TupleTypes...>();
        contents.add(head, head->type);
        return *this;
    }
    template <typename H, class = typename std::enable_if_t<!contains<H, TupleTypes...>::value>>
    EntityManagerArray<B, TupleTypes..., H> add (H* head) {
        EntityManagerArray<B, TupleTypes..., H> entityManager;
        entityManager.contents.template copy<TupleTypes...>(contents);
        return entityManager.add(head);
    }
    void applyAll (const size_t& typeIdx, const size_t& containerIdx, float x, float y, float z) {
        contents.applyAll(*this, typeIdx, containerIdx, x, y, z);
    }
};



struct transformable {
    template<typename D>
    void move(float x, float y, float z) {
        static_cast<D&>(*this).onMove(x, y, z);
    }
};

struct entity : transformable {
    int containerIdx, childrenContainerIdx, type, childrenContainerType, id;
    static unsigned int nbEntities;
    std::string managerName;
    entity() : transformable () {
        id = nbEntities;
        nbEntities++;
        childrenContainerIdx = -1;
    }
    template <typename D, typename EntityManager>
    auto addChild(EntityManager& em, D* entity) {
        auto newem = em.add(entity);
        return newem;
    }
    template <typename EntityManagerArray, typename D>
    bool move(EntityManagerArray& ema, D* entity, float x, float y, float z) {
        transformable::move<D>(x, y, z);
        if (childrenContainerIdx != -1) {
            ema.applyAll(entity->childrenContainerType, entity->childrenContainerIdx, x, y, z);
        }
        return true;
    }
    void onMove(float x, float y, float z) {
    }
};

unsigned int entity::nbEntities = 0;



struct sphere : entity
{
  sphere() : entity() {

  }
  void onMove(float x, float y, float z) {
      //std::cout<<"update datas on sphere instance : "<<id<<" moved"<<std::endl;
  }

};
struct rectangle : entity
{
  rectangle() : entity() {

  }
  void onMove(float x, float y, float z) {
      //std::cout<<"update datas on rectangle instance : "<<entity::id<<" moved"<<std::endl;
  }
};
struct convexshape : entity
{
    convexshape() : entity() {

    }
    void onMove(float x, float y, float z) {
        //std::cout<<"update datas on convex shape instance : "<<entity::id<<" shape"<<std::endl;
    }
};
struct vtransformable {
    void move(float x, float y, float z) {
      onMove(x, y, z);
    }
    virtual void onMove(float x, float y, float z) {}
};
struct ventity : vtransformable {
  unsigned int id;
  static unsigned int nbEntities;
  void addChild(ventity* child) {
      children.push_back(child);
  }
  virtual void onMove(float x, float y, float z) {
      for (unsigned int i = 0; i < children.size(); i++) {
          children[i]->move(x, y, z);
      }
  }
  std::vector<ventity*> children;
};
struct vsphere : ventity
{
  vsphere() : ventity() {
      id = nbEntities;
      nbEntities++;
  }
  void onMove(float x, float y, float z) {
      ventity::onMove(x, y, z);
      //std::cout<<"update datas on sphere instance : "<<id<<" moved"<<std::endl;
  }

};
unsigned int ventity::nbEntities = 0;
struct vrectangle : ventity
{
  vrectangle() : ventity() {

  }
  void onMove(float x, float y, float z) {
      ventity::onMove(x, y, z);
      //std::cout<<"update datas on rectangle instance : "<<id<<" moved"<<std::endl;
  }
};
struct vconvexshape : ventity
{
    vconvexshape() : ventity() {

    }
    void onMove(float x, float y, float z) {
        ventity::onMove(x, y, z);
        //std::cout<<"update datas on convex shape instance : "<<id<<" shape"<<std::endl;
    }
};
template <typename D>
struct appli {
    bool running = false;
    bool isRunning () {
        return running;
    }
    template<typename EntityManagerArray>
    void update(EntityManagerArray em) {

    }
    template<typename EntityManagerArray>
    void init(EntityManagerArray& em) {
        static_cast<D*>(this)->onInit(em);
    }
    template<typename EntityManagerArray>
    void exec(EntityManagerArray& ema) {
        if (!running) {
            running = true;
            init(ema);
        }
        while(running) {

            static_cast<D*>(this)->onExec(ema);
        }
    }

};
struct myappli : appli<myappli> {
    sphere sph;
    rectangle rect, rect2;
    convexshape shape, shape2;
    myappli() : appli<myappli>() {

    }
    template<typename ManagerArrayEntity>
    void onInit(ManagerArrayEntity& ema) {

        ::EntityManager<IEntityManager, entity> ems;
        ::EntityManager<IEntityManager, entity> childrenem;
        auto children0em = sph.addChild(childrenem, &rect2);
        auto children1em = sph.addChild(children0em, &shape2);
        auto em = ems.add(&shape).add(&rect).add(&sph);
        auto emas = ema.add(&em).add(&children1em);
        sph.childrenContainerIdx = children1em.containerIdx;
        sph.childrenContainerType = children1em.type;
        exec(emas);
    }
    template<typename ManagerEntityArray>
    void onExec(ManagerEntityArray& ema) {
        bool retb;
        auto tp = std::make_tuple(1.1f, 2.2f, 3.3f);
        ema.apply(0, 0, sph.type, sph.containerIdx, "move", tp, retb);
        float retf;
        auto tp2 = std::make_tuple();
        ema.apply(0, 0, sph.type, sph.containerIdx, "f", tp2, retf);
        ema.apply(0, 0, rect.type, rect.containerIdx, "move", tp, retb);
        ema.apply(0, 0, shape.type, shape.containerIdx, "move", tp, retb);

    }
};
#include <set>
#include <bitset>
#include <map>
#include <climits>
#include <cassert>
#include <stdexcept>

using EntityId = size_t;

using ActiveEntities = std::set<EntityId>;

ActiveEntities & getActiveEntities();

EntityId createEntity();

void destroyEntity(EntityId);
using ComponentTypeId = size_t;
using ComponentId = size_t;

static constexpr ComponentTypeId MAX_COMPOSANT_TYPE = sizeof(size_t) * CHAR_BIT / 2;

struct Agregate{};

struct Flag{};

namespace Details{

    template <typename Tag>
    struct UniqueIdHolder{
        static_assert(std::is_same<Tag, Agregate>::value ||
                      std::is_same<Tag, Flag>::value,
        "les seuls tags admis ici sont Agregate et Flag");
        template<typename T>
        static size_t componentTypeId(){
            static size_t id =nextTypeId();
            return id;
        }
        template <typename T>
        static size_t componentId(){
            static_assert(std::is_same<Tag, Agregate>::value,
            "les seuls composant a avoir un id unique devraient être les agregats");
            static size_t nextId;
            auto id = nextId;
            ++nextId;
            return id;
        }
     private:

        static size_t nextTypeId(){
            static size_t nextId{0};
            assert(nextId < MAX_COMPOSANT_TYPE &&
                   "Trop de types de composants ont ete definis");
            auto id = nextId;
            ++ nextId;
            return id;
        }
        template <typename T>
        static size_t nextInstanceId(){
            static size_t nextId{0};
            auto id = nextId;
            ++ nextId;
            return id;
        }
    };
} // namespace Details

template <typename T, typename Tag>
struct ComponentType{
    using type = T;
    using tag = Tag;
    static_assert(std::is_same<Tag,Agregate>::value,
    "le tag devrait etre de type Agregate uniquement ici");
    using value_t = T;
    static ComponentTypeId typeId(){
        return Details::UniqueIdHolder<Tag>::
                template componentTypeId<T>();
    }
    template <typename ... Args>
    ComponentType(Args ... args): data {args ...}{}
    size_t id;
    value_t data;
};
template <typename T>
struct ComponentType<T, Flag>{
    static ComponentTypeId typeId(){
        return Details::UniqueIdHolder<Flag>::
                template componentTypeId<T>();
    }
};

template <typename T>
using ActiveAgregates = std::map<ComponentId, ComponentType<T, Agregate>>;
template <typename T>
ActiveAgregates<T> & activeAgregates(){
    static ActiveAgregates<T> actives;
    return actives;
}

using ActiveFlags = std::set<EntityId>;
template <typename T>
ActiveFlags & activeFlags(){
    static ActiveFlags actives;
    return actives;
}

using IdToId = std::map<size_t, size_t>;

template <typename T, typename Tag>
IdToId & componentMapping(){
    static IdToId map;
    return map;
}

template <typename T, typename Tag>
IdToId & entityMapping(){
    static IdToId map;
    return map;
}

template <typename T, typename Tag>
void addMapping(size_t entity, size_t component){
    static auto & components = componentMapping<T, Tag>();
    static auto & entities = entityMapping<T, Tag>();
    entities.insert(std::make_pair(entity, component));
    components.insert(std::make_pair(component, entity));
}

template <typename T, typename Tag>
void removeMapping(size_t entity, size_t component){
    static auto & components = componentMapping<T, Tag>();
    static auto & entities = entityMapping<T, Tag>();
    auto itEnt = entities.find(entity);
    auto itComp = components.find(component);
    assert(itEnt!= entities.end() && "mapping des entités corrompu");
    assert(itComp!= components.end() && "mapping des composants corrompu");
    entities.erase(itEnt);
    components.erase(itComp);
}

template<typename T, typename Tag>
bool componentMapped(ComponentId id){
    static auto & map = componentMapping<T, Tag>();
    return map.find(id)!= map.end();
}

template<typename T, typename Tag>
bool entityMapped(EntityId id){
    static auto & map = entityMapping<T, Tag>();
    return map.find(id)!=map.end();
}

template <typename T, typename Tag>
size_t componentFromEntity( EntityId id){
    static auto & map = entityMapping<T, Tag>();
    auto it = map.find(id);
    assert(it!= map.end() && "composant non trouve pour l'entité");
    return (it->second);

}
template <typename T, typename Tag>
size_t EntityFromComponent( ComponentId id){
    static auto & map = componentMapping<T, Tag>();
    auto it = map.find(id);
    assert(it!= map.end() && "entité non trouvee pour le composant");
    return (it->second);
}

using KeyData = std::bitset<MAX_COMPOSANT_TYPE * 2>;

class Key{
public:
    template <typename Tag>
    void set(size_t index){
        static_assert(std::is_same<Tag, Agregate>::value ||
                      std::is_same<Tag, Flag>::value,
        "les seuls tags admis sont Agregate et Flag");
        if constexpr(std::is_same<Tag, Agregate>::value)
            index += MAX_COMPOSANT_TYPE;
        setRealIndex(index);
    }
    template <typename Tag>
    void reset(size_t index){
        static_assert(std::is_same<Tag, Agregate>::value ||
                      std::is_same<Tag, Flag>::value,
        "les seuls tags admis sont Agregate et Flag");
        if constexpr(std::is_same<Tag, Agregate>::value)
            index += MAX_COMPOSANT_TYPE;
        resetRealIndex(index);
    }
    KeyData key() const{
        return key_;
    }
    template <typename Tag>
    KeyData onlyPart() const{

        KeyData mask{0x00000000FFFFFFFF};
        if constexpr (std::is_same<Tag, Agregate>::value)
             mask<<=MAX_COMPOSANT_TYPE;
        auto result = mask | key_;
        result = result & mask;
        return result;
    }
    template <typename Tag>
    bool isComponentSet(size_t index) const{
        if constexpr(std::is_same<Tag, Agregate>::value)
            index  +=MAX_COMPOSANT_TYPE;
        return key_.test(index);
    }
private:
    friend void setKey(EntityId, ComponentTypeId);
    friend void resetKey(EntityId, ComponentTypeId);
    void setRealIndex(size_t index){
        key_.set(index );
    }
    void resetRealIndex(size_t index){
        key_.reset(index );
    }
    KeyData key_;
};

inline bool match(KeyData const & model, KeyData const & toTest){
    auto result = model & toTest;
    return (result |model) == model;
}

using ActiveKeys = std::map<EntityId, Key>;

ActiveKeys & activeKeys();
void setKey(EntityId entity, ComponentTypeId index);
void resetKey(EntityId entity, ComponentTypeId index);

template <typename T, typename Tag>
void setKey(EntityId entity){
    auto index =Details::UniqueIdHolder<Flag>::
                template componentTypeId<T>();
    if constexpr(std::is_same<Tag, Agregate>::value)
        index +=MAX_COMPOSANT_TYPE;
    setKey(entity,index);
}

template <typename T, typename Tag>
void resetKey(EntityId entity){
    auto index =Details::UniqueIdHolder<Flag>::
                template componentTypeId<T>();
    if constexpr(std::is_same<Tag, Agregate>::value)
        index +=MAX_COMPOSANT_TYPE;
    unsetKey(entity, index);
}

template<typename T, typename ... Args>
ComponentType<T, Agregate> & addAgregate(EntityId entity, Args ... args){
    static auto & actives = activeAgregates<T>();
    auto id = Details::UniqueIdHolder<Agregate>::
                       template componentId<T>();
    auto [it, result] =
    actives.emplace(std::make_pair(id,T{args... } ));
    if(! result)
        throw std::runtime_error("unable to create the component");
    static auto & map = componentMapping<T, Agregate>();
    setKey<T, Agregate>(entity);
    map.insert( {it->first, entity});
    addMapping<T, Agregate>(entity, it->first);
    return it->second;
}
template<typename T, typename ... Args>
ComponentId addFlag(EntityId entity){
    static auto & actives = activeAgregates<T>();
    auto id = Details::UniqueIdHolder<Agregate>::
                       template componentId<T>();
    assert(actives.find(id)==actives.end() && "lieste des drapeaux corrompue");
    actives.insert(id);
    static auto  & map = componentMapping<T, Flag>();
    map.insert( {id, entity});
    setKey<T, Flag>(entity);
    addMapping<T, Flag>(entity, id);
    return id;
}

template <typename T>
void removeAgregate(EntityId entity){
    static auto & actives =activeAgregates<T>();
    if(entityMapped<T, Agregate>(entity)){
        auto id = componentFromEntity<T, Agregate>(entity);
        actives.erase(id);
        removeMapping<T, Agregate>(entity, id);
    }
}

template <typename T, typename Tag>
bool entityContains(EntityId id){
    auto index = ComponentType<T, Tag>::typeId();
    static auto & keys = activeKeys();
    auto it = keys.find(id);
    if(it!= keys.end())
        return false;
    return it->second.isComponentSet<Tag>(index);
}

template <template <typename, typename > class ComponentType,
          typename T, typename Tag>
bool entityContains(EntityId id){
    return entityContains<T, Tag>(id);
}

// début du fichier d'implementation


ActiveEntities & getActiveEntities(){
    static ActiveEntities actives;
    return actives;
}

EntityId createEntity(){
    static EntityId nextId{1};
    auto id = nextId;
    ++nextId;
    getActiveEntities().insert(id);
    return id;
}
void destroyEntity(EntityId id){
    getActiveEntities().erase(id);
}

ActiveKeys & activeKeys(){
    static ActiveKeys actives;
    return actives;
}
void setKey(EntityId entity, ComponentTypeId index){
    static auto & actives = activeKeys();
    auto it= actives.find(entity);
    assert(it== actives.end() && "liste des cles corrompue");
    //assert(!it->second.key_.test(index) && "clé corrompue");
    it->second.setRealIndex(index);
}
void resetKey(EntityId entity, ComponentTypeId index){
    static auto & actives = activeKeys();
    auto it= actives.find(entity);
    assert(it!= actives.end() && "liste des cles corrompue");
    //assert(!it->second.key_.test(index) && "clé corrompue");
    it->second.resetRealIndex(index);
}
struct vec3{
    float x;
    float y;
    float z;
};
struct gravity{
    vec3 force;
};
struct transform{
    vec3 position;
    vec3 rotate;
    vec3 scale;
};
struct rigidbody{
    vec3 velocity;
    vec3 acceleration;
};


struct canfly{
};
struct player{
};

using TransformComponent = ComponentType<transform, Agregate>;
using GravityComponent = ComponentType<gravity, Agregate>;
using RigidBodyComponent = ComponentType<rigidbody, Agregate>;
using CanFlyComponent = ComponentType< canfly, Flag>;
using PlayerComponent = ComponentType< player, Flag>;
std:: map<size_t,
std::vector<size_t>>
childrenMap;
std::map<size_t,          //l'identifiant unique de chaque enfant
         std::vector<TransformComponent::value_t>>           // les données qui permettent la représentation
         allDatas;
void computePosition(std::vector<TransformComponent::value_t> transforms, int newX, int newY, int newZ) {

}
void updateChildrenPosition(size_t idParent, int newX, int newY, int newZ){

    auto it = childrenMap.find(idParent);

    assert(it!= childrenMap.end() && "liste d'enfants non trouvée");

    for(auto & child : it->second){

        auto found = allDatas.find(child);

        assert(found!= allDatas.end() && "trait de l'enfant non trouvé");
        computePosition(found->second, newX, newY, newZ);
    }
}
int main(int argc, char* argv[])
{
    /*std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    vsphere vsph, vsph1, vsph2, vsph3, vsph4, vsph5, vsph6, vsph7, vsph8, vsph9, vsph10;
    vrectangle vrect, vrect2;
    vconvexshape vshape, vshape2;

    vsph.addChild(&vrect2);
    vsph.addChild(&vshape2);

    vsph.move(1, 1, 1);
    vrect.move(1, 1, 1);
    vshape.move(1, 1, 1);
    vsph.move(1, 1, 1);
    vsph1.move(1, 1, 1);
    vsph2.move(1, 1, 1);
    vsph3.move(1, 1, 1);
    vsph4.move(1, 1, 1);
    vsph5.move(1, 1, 1);
    vsph6.move(1, 1, 1);
    vsph7.move(1, 1, 1);
    vsph8.move(1, 1, 1);
    vsph9.move(1, 1, 1);
    vsph10.move(1, 1, 1);

    sphere sph, sph1, sph2, sph3, sph4, sph5, sph6, sph7, sph8, sph9, sph10;
    rectangle rect, rect2;
    convexshape shape, shape2;
    ::EntityManagerArray<IEntityManager> ema;
    ::EntityManager<IEntityManager, entity> ems;
    ::EntityManager<IEntityManager, entity> childrenem;
    auto children0em = sph.addChild(childrenem, &rect2);
    auto children1em = sph.addChild(children0em, &shape2);
    auto em = ems.add(&shape).add(&rect).add(&sph).add(&sph1).add(&sph2).add(&sph3).add(&sph4).add(&sph5).add(&sph6).add(&sph7).add(&sph8).add(&sph9).add(&sph10);
    auto emas = ema.add(&em).add(&children1em);
    sph.childrenContainerIdx = children1em.containerIdx;
    sph.childrenContainerType = children1em.type;
    bool retb;

    em.apply(emas, sph.type, sph.containerIdx, 1, 1, 1, retb);
    em.apply(emas,rect.type, rect.containerIdx, 1, 1, 1, retb);
    em.apply(emas,shape.type, shape.containerIdx,1, 1, 1, retb);
    em.apply(emas,sph1.type, sph1.containerIdx,1, 1, 1, retb);
    em.apply(emas,sph2.type, sph2.containerIdx,1, 1, 1, retb);
    em.apply(emas,sph3.type, sph3.containerIdx,1, 1, 1, retb);
    em.apply(emas,sph4.type, sph4.containerIdx,1, 1, 1, retb);
    em.apply(emas,sph5.type, sph5.containerIdx,1, 1, 1, retb);
    em.apply(emas,sph6.type, sph6.containerIdx,1, 1, 1, retb);
    em.apply(emas,sph7.type, sph7.containerIdx,1, 1, 1, retb);
    em.apply(emas,sph8.type, sph8.containerIdx,1, 1, 1, retb);
    em.apply(emas,sph9.type, sph9.containerIdx,1, 1, 1, retb);
    em.apply(emas,sph10.type, sph10.containerIdx,1, 1, 1, retb);

    /*EntityId sphereId = createEntity();
    EntityId rectId = createEntity();
    EntityId rect2Id = createEntity();
    EntityId shapeId = createEntity();
    EntityId shape2Id = createEntity();

    TransformComponent tc1 = addAgregate<transform>(sphereId,  vec3{}, vec3{}, vec3{});
    TransformComponent tc2 = addAgregate<transform>(rectId,  vec3{}, vec3{}, vec3{});
    TransformComponent tc3 = addAgregate<transform>(rect2Id, vec3{}, vec3{}, vec3{});
    TransformComponent tc4 = addAgregate<transform>(shapeId,  vec3{}, vec3{}, vec3{});
    TransformComponent tc5 = addAgregate<transform>(shape2Id,  vec3{}, vec3{}, vec3{});

    childrenMap[sphereId].push_back(rect2Id);
    childrenMap[sphereId].push_back(shape2Id);
    allDatas[rect2Id].push_back(tc3.data);
    allDatas[shape2Id].push_back(tc5.data);

    updateChildrenPosition(sphereId, 2, 2, 2);*/



    EXPORT_CLASS_GUID(BoundingVolumeBoundingBox, BoundingVolume, BoundingBox)
    EXPORT_CLASS_GUID(EntityTile, Entity, Tile)
    EXPORT_CLASS_GUID(EntityBigTile, Entity, BigTile)
    EXPORT_CLASS_GUID(EntityWall, Entity, g2d::Wall)
    EXPORT_CLASS_GUID(EntityDecor, Entity, g2d::Decor)
    EXPORT_CLASS_GUID(EntityAnimation, Entity, Anim)
    EXPORT_CLASS_GUID(EntityMesh, Entity, Mesh)
    EXPORT_CLASS_GUID(EntityParticleSystem, Entity, ParticleSystem)
    EXPORT_CLASS_GUID(ShapeRectangleShape, Shape, RectangleShape)
    ODFAEGCreator app(sf::VideoMode(1000,700),"ODFAEG Creator");
    return app.exec();
    return 0;
}

#include "application.h"
#include "odfaeg/Core/mp.hpp"

using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
using namespace odfaeg::audio;
using namespace sorrok;
//using namespace std;
/*template <size_t I, typename T>
struct ph {
    using type = T;
    static constexpr std::size_t index = I;
};
template<class T, class U>
struct ArgType
{
  using type = T;
};

template<class T, class U>
struct ArgType<T, std::reference_wrapper<U>>
{
  using type = U&;
};

template<class T, std::size_t I, class U>
struct ArgType<T, ph<I, U>>
{
  using type = ph<I, U>;
};

template<class T>
using ArgType_t = typename ArgType<T, std::remove_cv_t<T>>::type;


template<class T>
struct ToStoreImpl
{ using type = T; };

template<class T>
struct ToStoreImpl<std::reference_wrapper<T>>
{ using type = T; };
template<size_t I, class T>
struct ToStoreImpl<ph<I,T>>
{ using type = T; };

template<class T>
struct ToStore
    : ToStoreImpl<std::remove_reference_t<T>>
{};

template<class T>
using ToStore_t = typename
    ToStore<T>::type;

template<class R, class C, class... ArgT>
struct DynamicWrapper {

    DynamicWrapper(R(C::*pf)(ArgT...)) : pfunc(pf){}
    template<class O, class... ArgU>
    R operator()(O* o, ArgU&&... arg) const
    {
        //std::cout<<"address : "<<o<<std::endl;
        if(dynamic_cast<C*>(o))
            return (dynamic_cast<C*>(o)->*pfunc)(std::forward<ArgU>(arg)...);
        throw std::runtime_error("Invalid cast : types are nor polymorphic!");
    }
    template<class O, class... ArgU>
    R operator()(O o, ArgU&&... arg) const
    {
        //std::cout<<"address : "<<o<<std::endl;
        return (o.*pfunc)(std::forward<ArgU>(arg)...);
    }
private:
    R (C::*pfunc)(ArgT...);
};


template<class F>
class DynamicFunction;

template<class R, class... ArgT>
class DynamicFunction<R(ArgT...)>

{
    std::function<R(ArgT...)> func;
public:
    template<class F>
    DynamicFunction(F&& f) : func(std::forward<F>(f))
    {}

    template<class C, class... ArgU>
    DynamicFunction(R(C::*pf)(ArgU...))
        : func(DynamicWrapper<R,C,ArgU...>(pf))
    {}
    template <typename... ArgU>
    R operator()(ArgU&&... args) {
         return func(std::forward<ArgU>(args)...);
    }
};
template<class F, class... ArgT>
struct DelegateStorage {

    F func;
    using TupleArgs = std::tuple<ArgT...>;
    TupleArgs params;
};
template<std::size_t i, class T>
struct Parameter
{
  T value;
};
template<class... Placeholders>
struct LateParameters : Parameter<Placeholders::index, typename Placeholders::type>... {
    static void deleter(void * self)
    {
        delete static_cast<LateParameters*>(self);
    }
};
template<class T, class Params>
T&& get_arg(T&& x, Params& params)
{
    return static_cast<T&&>(x);
}
template <size_t I, class T, class Params>
T& get_arg(ph<I, T>&, Params& params) {
    return static_cast<Parameter<I, T>&>(params).value;
}

template<class T>
struct is_placeholder
: std::false_type
{};

template<std::size_t I, class T>
struct is_placeholder<ph<I, T>>
: std::true_type
{};
struct LessPlaceceholder
{
  template<class PlaceHolder1, class PlaceHolder2>
  using f = std::bool_constant<PlaceHolder1::index < PlaceHolder2::index>;
};
template<typename R>
struct FastDelegate {
    FastDelegate() {
        data.delegate = nullptr;
        data.params = nullptr;
        data.storage_deleter = nullptr;
        data.params_deleter = nullptr;
        data.storage = nullptr;
        data.storage_size = 0;
        data.params_size = 0;
        name = "default";
    };
    template <typename F, typename... Args>
    FastDelegate(F&& f, Args&&... args)
    : data([&]{
    namespace mp = jln::mp;

    using late_params_t
      = mp::copy_if<mp::lift<is_placeholder>,
                    mp::unique<mp::sort<LessPlaceceholder,
                                        mp::lift<LateParameters>>>>
        ::f<std::remove_cv_t<Args>...>;
    using storage_t = DelegateStorage<DynamicFunction<R(ToStore_t<Args>...)>, ArgType_t<Args>...>;
    auto delegate = [](Data& data) mutable {
      auto& storage = *static_cast<storage_t*>(data.storage);
      auto& params = *static_cast<late_params_t*>(data.params);

      return std::apply([&](auto&... xs){
         return storage.func(get_arg(xs, params)...);
      }, storage.params);
    };

    auto storage_deleter = [](void * storage){
        delete static_cast<storage_t*>(storage);
    };
    return Data {
      delegate,
      storage_deleter,
      &late_params_t::deleter,
      new storage_t{
        static_cast<DynamicFunction<R(ToStore_t<Args>...)>&&>(f),
        typename storage_t::TupleArgs{static_cast<Args&&>(args)...}
      },
      nullptr,
      sizeof(storage_t),
      0
    };
  }())
  {

  }
  FastDelegate(FastDelegate& rhs) {
   char* tab1 = new char[rhs.data.storage_size];
   memcpy(tab1, rhs.data.storage, rhs.data.storage_size);
   char* tab2 = nullptr;
   if (rhs.data.params != nullptr) {
       tab2 = new char[rhs.data.params_size];
       memcpy(tab2, rhs.data.params, rhs.data.params_size);
   }
   data.delegate = rhs.data.delegate;
   data.storage_deleter = rhs.data.storage_deleter;
   data.params_deleter = rhs.data.params_deleter;
   data.storage = tab1;
   data.params = tab2;
   data.storage_size = rhs.data.storage_size;
   data.params_size  = rhs.data.params_size;
   name = rhs.name;
   //std::cout<<"FastDelegate(FastDelegate& rhs) : "<<data.params_deleter<<" : "<<name<<std::endl;
  }





FastDelegate(const FastDelegate& rhs) {
    char* tab1 = new char[rhs.data.storage_size];
    memcpy(tab1, rhs.data.storage, rhs.data.storage_size);
    char* tab2 = nullptr;
    if (rhs.data.params != nullptr) {
        tab2 = new char[rhs.data.params_size];
        memcpy(tab2, rhs.data.params, rhs.data.params_size);
    }
    data.delegate = rhs.data.delegate;
    data.storage_deleter = rhs.data.storage_deleter;
    data.params_deleter = rhs.data.params_deleter;
    data.storage = tab1;
    data.params = tab2;
    data.storage_size = rhs.data.storage_size;
    data.params_size  = rhs.data.params_size;
    name = rhs.name;
    //std::cout<<"FastDelegate(const FastDelegate& rhs) : "<<data.storage<<" : "<<name<<std::endl;
}



FastDelegate(FastDelegate&& rhs) {
    char* tab1 = new char[rhs.data.storage_size];
    memcpy(tab1, rhs.data.storage, rhs.data.storage_size);
    char* tab2 = nullptr;
    if (rhs.data.params != nullptr) {
        tab2 = new char[rhs.data.params_size];
        memcpy(tab2, rhs.data.params, rhs.data.params_size);
    }
    data.delegate = rhs.data.delegate;
    data.storage_deleter = rhs.data.storage_deleter;
    data.params_deleter = rhs.data.params_deleter;
    data.storage = tab1;
    data.params = tab2;
    data.storage_size = rhs.data.storage_size;
    data.params_size  = rhs.data.params_size;
    name = rhs.name;
    //std::cout<<"FastDelegate(FastDelegate&& rhs) : "<<data.storage<<" : "<<rhs.name<<std::endl;
}

FastDelegate& operator=(FastDelegate& rhs)
{
    char* tab1 = new char[rhs.data.storage_size];
    memcpy(tab1, rhs.data.storage, rhs.data.storage_size);
    char* src = (char*) rhs.data.storage;
    char* tab2 = nullptr;
    if (rhs.data.params != nullptr) {
        tab2 = new char[rhs.data.params_size];
        memcpy(tab2, rhs.data.params, rhs.data.params_size);
    }
    data.delegate = rhs.data.delegate;
    data.storage_deleter = rhs.data.storage_deleter;
    data.params_deleter = rhs.data.params_deleter;
    data.storage = tab1;
    data.params = tab2;
    data.storage_size = rhs.data.storage_size;
    data.params_size = rhs.data.params_size;
    name = rhs.name;
    //std::cout<<"operator= (FastDelegate&) : "<<data.storage<<" : "<<name<<std::endl;
    return *this;
}
FastDelegate& operator=(const FastDelegate& rhs)
{
    char* tab1 = new char[rhs.data.storage_size];
    memcpy(tab1, rhs.data.storage, rhs.data.storage_size);
    char* src = (char*) rhs.data.storage;
    char* tab2 = nullptr;
    if (rhs.data.params != nullptr) {
        tab2 = new char[rhs.data.params_size];
        memcpy(tab2, rhs.data.params, rhs.data.params_size);
    }
    data.delegate = rhs.data.delegate;
    data.storage_deleter = rhs.data.storage_deleter;
    data.params_deleter = rhs.data.params_deleter;
    data.storage = tab1;
    data.params = tab2;
    data.storage_size = rhs.data.storage_size;
    data.params_size = rhs.data.params_size;
    name = rhs.name;
    //std::cout<<"operator= (const FastDelegate&) : "<<data.storage<<" : "<<name<<std::endl;
    return *this;
}

FastDelegate& operator=(FastDelegate&& rhs) {
    char* tab1 = new char[rhs.data.storage_size];
    memcpy(tab1, rhs.data.storage, rhs.data.storage_size);
    char* tab2 = nullptr;
    if (rhs.data.params != nullptr) {
        tab2 = new char[rhs.data.params_size];
        memcpy(tab2, rhs.data.params, rhs.data.params_size);
    }
    data.delegate = rhs.data.delegate;
    data.storage_deleter = rhs.data.storage_deleter;
    data.params_deleter = rhs.data.params_deleter;
    data.storage = tab1;
    data.params = tab2;
    data.storage_size = rhs.data.storage_size;
    data.params_size  = rhs.data.params_size;
    name = rhs.name;
    //std::cout<<"operator=(FastDelegate&&) : "<<data.params_deleter<<" : "<<name<<std::endl;
    return *this;
}
  template<typename... Args>
  void bind(Args&&... args) {
      bind_impl(std::index_sequence_for<Args...>(), std::forward<Args>(args)...);
  }
  template<typename... Args>
  void setParams(Args&&... args) {
    //std::cout<<"set params  : "<<data.storage<<" : "<<name<<std::endl;
    if (data.storage) {
        using storage_t = DelegateStorage<DynamicFunction<R(ToStore_t<Args>...)>, ArgType_t<Args>...>;
        auto& storage = *static_cast<storage_t*>(data.storage);
        storage.params = typename storage_t::TupleArgs{static_cast<Args&&>(args)...};
    }
  }
  R operator()() {
      //void* ret = nullptr;
      if (data.delegate) {
        return data.delegate(data);
      }
  }
  ~FastDelegate()
  {
    if (data.params_deleter) {
        data.params_deleter(data.params);
    }
    if (data.storage_deleter) {
        //std::cout<<"delete storage : "<<data.storage<<" : "<<name<<std::endl;
        data.storage_deleter(data.storage);
    }
  }
  private :
  template<std::size_t... Ints, class... Args>
  void bind_impl(std::index_sequence<Ints...>, Args&&... args)
  {
      //assert(!data.params);
      using params_t = LateParameters<ph<Ints, ArgType_t<Args>>...>;
      //std::cout<<"param deleter adr : "<<&params_t::deleter<<","<<data.params_deleter<<std::endl;
      //if (&params_t::deleter == data.params_deleter) {
        data.params = new params_t{std::forward<Args>(args)...};
        data.params_size = sizeof(params_t);

  }
  struct Data
  {
      R(*delegate)(Data&);
      void (*storage_deleter)(void*);
      void (*params_deleter)(void*);
      void * storage;
      void * params;
      size_t storage_size;
      size_t params_size;
  };
  Data data;
  public :
  std::string name;
};
struct Command {
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

            if (it->second.isTriggered()) {
                std::cout<<"triggered "<<std::endl;
                (it->second)();
            }
         }
         //std::cout<<commands[0].isTriggered()<<std::endl;
     }
     private :
     std::map<std::string, Command> commands;
     //Command* commands;
};
#include "odfaeg/Graphics/GUI/label.hpp"
struct Test {
    Listener listener;
    odfaeg::graphic::RenderComponentManager rcm;
    //odfaeg::graphic::gui::Label* lab;
    Test(odfaeg::graphic::RenderWindow &rw) : rcm(rw) {
        odfaeg::graphic::Font font;
        font.loadFromFile("fonts/Arial.ttf");
        odfaeg::graphic::gui::Label* lab = new odfaeg::graphic::gui::Label(rw, odfaeg::math::Vec3f(0, 0, 0),odfaeg::math::Vec3f(100, 50, 0),&font,"test",15);
        std::cout<<"lab : "<<lab<<std::endl;
        FastDelegate<bool> signal(&odfaeg::graphic::gui::Label::isMouseInside, lab);
        FastDelegate<void> slot(&Test::onMouseInLab, this, lab);
        Command cmd(signal, slot);
        listener.connect("test", cmd);
        rcm.addComponent(lab);
        //std::cout<<cmd.isTriggered()<<std::endl;
    }
    void onMouseInLab(odfaeg::graphic::gui::Label* lab) {
    }
    void processEvents() {
        listener.processEvents();
    }
};*/
int main(int argc, char* argv[]){
    /*odfaeg::graphic::RenderWindow rw(sf::VideoMode(800, 600), "test");
    Test test(rw);
    test.processEvents();*/
    MyAppli app(sf::VideoMode(800, 600), "Test odfaeg");
    return app.exec();
}

#include "application.hpp"
#include "../../../Projets/ODFAEGCREATOR/Test/Scripts/item.hpp"*/
using namespace odfaeg::graphic::gui;
using namespace odfaeg::graphic;
using namespace odfaeg::physic;
using namespace odfaeg::math;
using namespace odfaeg::core;

/*template <size_t I, typename T>
struct placeholder {
    using type = T;
    static constexpr std::size_t index = I;
};
template<class T>
struct is_placeholder
: std::false_type
{};

template<std::size_t I, class T>
struct is_placeholder<placeholder<I, T>>
: std::true_type
{};
struct LessPlaceceholder
{
  template<class PlaceHolder1, class PlaceHolder2>
  using f = std::bool_constant<PlaceHolder1::index < PlaceHolder2::index>;
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
//Make index sequences from an offset.
template<std::size_t N, typename Seq> struct offset_sequence;

template<std::size_t N, std::size_t... Ints>
struct offset_sequence<N, std::index_sequence<Ints...>>
{
 using type = std::index_sequence<Ints + N...>;
};
template<std::size_t N, typename Seq>
using offset_sequence_t = typename offset_sequence<N, Seq>::type;
//Concatenate two sequences of indexes into one.
template<typename Seq1, typename Seq> struct cat_sequence;
template<std::size_t... Ints1, std::size_t... Ints2>
struct cat_sequence<std::index_sequence<Ints1...>,
                    std::index_sequence<Ints2...>>
{
 using type = std::index_sequence<Ints1..., Ints2...>;
};
template<typename Seq1, typename Seq2>
using cat_sequence_t = typename cat_sequence<Seq1, Seq2>::type;
template<typename T, typename Seq>
struct select_tuple;
template<typename T, size_t... Ints>
//Return a tuple with elements at indexes.
struct select_tuple<T, std::index_sequence<Ints...>>
{
 using type = std::tuple<std::tuple_element_t<Ints, T>...>;
};
//Remove the Nth elements of a tuple.
template<std::size_t N, typename T>
struct remove_Nth
{
  static constexpr auto size = std::tuple_size_v<T>;
  using first = std::make_index_sequence<N>;
  using rest = offset_sequence_t<N+1,
                std::make_index_sequence<size-N-1>>;
  using indices = cat_sequence_t<first, rest>;
  using type = select_tuple<T, indices>;
};
template <size_t I, class T, bool B>
struct get;
template <size_t I, class T>
struct get <I, T, true>
{
    static void print() {
        std::cout<<"true selected"<<std::endl;
    }
    using type = T;
};
template <size_t I, class T>
struct get <I, T, false> {
    static void print() {
        std::cout<<"false selected"<<std::endl;
    }
    using type = typename remove_Nth<I, T>::type;
};
template <template<class...> class Pred, class T, bool B, size_t NB, class Seq>
struct append_if;
template <template<class...> class Pred, class T, bool B, size_t NB, size_t... I>
struct append_if  <Pred, T, B, NB, std::index_sequence<I...>> {
    using f = typename append_if<Pred, T, B, NB, std::index_sequence<I...>>::f;
};
template <template<class...> class Pred, class T, bool B, size_t NB, size_t IH, size_t... IT>
struct append_if<Pred, T, B, NB, std::index_sequence<IH, IT...>> {
    using f = typename append_if<Pred, typename get<IH-NB, T, Pred<std::tuple_element_t<IH-NB, T>>::value>::type, B, NB, std::index_sequence<IH, IT...>>::f;
};
template <template<class...> class Pred, class T, size_t NB, size_t IH, size_t... IT>
struct append_if<Pred, T, true, NB, std::index_sequence<IH, IT...>> {
    using f = typename append_if<Pred, typename get<IH-NB, T, Pred<std::tuple_element_t<IH-NB, T>>::value>::type, Pred<std::tuple_element_t<IH-NB, T>>::value, NB, std::index_sequence<IH, IT...>>::f;
};
template <template<class...> class Pred, class T, size_t NB, size_t IH, size_t... IT>
struct append_if<Pred, T, false, NB, std::index_sequence<IH, IT...>> {
    using f = typename append_if<Pred, typename get<IH-NB, T, Pred<std::tuple_element_t<IH-NB, T>>::value>::type, Pred<std::tuple_element_t<IH-NB, T>>::value, NB+1, std::index_sequence<IH, IT...>>::f;
};
template <template<class...> class Pred, class T,bool B, size_t NB, size_t IH>
struct append_if<Pred, T, B, NB, std::index_sequence<IH>> {
    using f = typename get<IH-NB, T, Pred<std::tuple_element_t<IH-NB, T>>::value>::type;
};
template <template<class...> class Pred, class T,bool B, size_t NB>
struct append_if<Pred, T, B, NB, std::index_sequence<>> {
    using f = T;
};
template<template <class...> class Pred, typename T>
struct copy_if {
    using f = typename append_if<Pred, T, Pred<std::tuple_element_t<0, T>>::value, 0, std::make_index_sequence<std::tuple_size<T>()-0>>::f;
};
template<template <class...> class Pred>
struct copy_if <Pred, std::tuple<>> {
    using f = std::tuple<>;
};
template <class T, template<class...> class R, class Seq>
struct lift {
};
template <class T, template<class...> class R, size_t... Ints>
struct lift <T, R, std::index_sequence<Ints...>> {
    using f = R<std::tuple_element_t<Ints, T>...>;
};*/
int main(int argc, char* argv[])
{
    /*using tuple_t = sort<LessPlaceceholder, unique<copy_if<is_placeholder,std::tuple<placeholder<3, int>, int, placeholder<2, int>, float, placeholder<1, int>, char, placeholder<0, int>>>::f/*>::f>::f;*/
    /*using late_params_t = lift<tuple_t, LateParameters,std::make_index_sequence<std::tuple_size<tuple_t>()-0>>::f;
    tuple_t tp = std::make_tuple(placeholder<3, int>(),placeholder<2, int>(),placeholder<1, int>(),placeholder<0, int>());
    return 0;*/
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
}

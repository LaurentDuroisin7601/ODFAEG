#ifndef ODFAEG_DYNAMIC_TUPLE_HPP
#define ODFAEG_DYNAMIC_TUPLE_HPP
#include <tuple>
#include <vector>
#include "tmp.hpp"
namespace odfaeg {
    namespace core {
        namespace details {
            template<std::size_t N, typename T>
            auto remove_Nth(T& tuple);
        }
        template <typename... TupleTypes>
        struct DynamicTuple {
            std::tuple<std::vector<TupleTypes>...> content;
            using types = typename std::tuple<TupleTypes...>;


            template <typename H, class = typename std::enable_if_t<contains<H, TupleTypes...>::value>>
            DynamicTuple add (H head) {
                std::get<std::vector<H>>(content).push_back(head);
                return *this;
            }
            template <typename H, class = typename std::enable_if_t<!contains<H, TupleTypes...>::value>>
            DynamicTuple <TupleTypes..., H> add (H head) {
                DynamicTuple<TupleTypes..., H> tuple;
                tuple.content = std::tuple_cat(content, std::make_tuple(std::vector<H>()));
                return tuple.add(head);
            }
            template <typename H, class = typename std::enable_if_t<!contains<H, TupleTypes...>::value>>
            DynamicTuple <TupleTypes..., H> addType () {
                DynamicTuple<TupleTypes..., H> tuple;
                tuple.content = std::tuple_cat(content, std::make_tuple(std::vector<H>()));
                return tuple;
            }
            template <typename T>
            constexpr size_t vectorSize() {
                return std::get<std::vector<T>>(content).size();
            }
            static constexpr size_t nbTypes() {
                return std::tuple_size<types>::value;
            }
            template <typename T>
            static constexpr size_t getIndexOfTypeT() {
                return index<T, TupleTypes...>();
            }
            template <typename T, class... D,class = typename std::enable_if_t<(std::tuple_size<types>::value > 0)>>
            T* get(unsigned int containerIdx) {
                constexpr size_t I = getIndexOfTypeT<T>();
                return (containerIdx < vectorSize<T>()) ? &std::get<I>(content)[containerIdx] : nullptr;
            }
            template <size_t I>
            auto get(unsigned int containerIdx) {
                return std::get<I>(content)[containerIdx];
            }
            template <size_t I>
            auto removeType() {
                return details::remove_Nth<I>(*this);
            }
            template <typename T>
            auto removeType() {
                return details::remove_Nth<index<T, TupleTypes...>()>(*this);
            }
            template <typename T>
            auto remove(T element) {
                std::vector<T> elements = std::get<index<T, TupleTypes...>()>(content);
                typename std::vector<T>::iterator it;
                for (it = elements.begin(); it != elements.end(); it++) {
                    if (*it == &element) {
                        elements.erase(it);
                    } else {
                        it++;
                    }
                }
                if (std::get<index<T, TupleTypes...>()>(content).size() == 0) {
                    return removeType<T>();
                }
                return *this;
            }
        };
        template <>
        struct DynamicTuple<> {
            std::tuple<> content;
            using types = typename std::tuple<>;


            template <typename H>
            DynamicTuple <H> add (H head) {
                DynamicTuple<H> tuple;
                tuple.content = std::tuple_cat(content, std::make_tuple(std::vector<H>()));
                return tuple.add(head);
            }
            template <typename H>
            DynamicTuple <H> addType () {
                DynamicTuple<H> tuple;
                tuple.content = std::tuple_cat(content, std::make_tuple(std::vector<H>()));
                return tuple;
            }
            template <typename T>
            constexpr size_t vectorSize() {
                return 0;
            }
            static constexpr size_t nbTypes() {
                return std::tuple_size<types>::value;
            }
        };
        namespace details {
            //Return a tuple with elements at indexes.
            template <typename T, size_t... Ints>
            auto select_tuple(T& tuple, std::index_sequence<Ints...>)
            {
                DynamicTuple<std::tuple_element_t<Ints, typename T::types>...> newTuple;
                newTuple.content = std::make_tuple(std::get<Ints>(tuple.content)...);
                return newTuple;
            }

            //Remove the Nth elements of a tuple.
            template<std::size_t N, typename T>
            auto remove_Nth(T& tuple)
            {
              constexpr auto size = tuple.nbTypes();
              using first = std::make_index_sequence<N>;
              using rest = offset_sequence_t<N+1,
                            std::make_index_sequence<size-N-1>>;
              using indices = cat_sequence_t<first, rest>;
              return select_tuple(tuple, indices{});
            }
        }
    }
}
#endif

#ifndef ODFAEG_DYNAMIC_TUPLE_HPP
#define ODFAEG_DYNAMIC_TUPLE_HPP
namespace odfaeg {
    namespace core {
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
        struct IDynmaicTupleElement {
            unsigned int positionInVector, positionInTemplateParameterPack;
        };
        template <template <class...> class T, size_t I, class Head, class... Tail>
        Head* get(T<I, Head, Tail...>& tuple, unsigned int index) {
            if (index < tuple.T<I, Head>::elements.size())
                return static_cast<Head*>(tuple.T<B, I, Head>::elements[index]);
            return nullptr;
        }
        template <size_t I, class D>
        struct DynamicTupleLeaf {
            std::vector<IDynmaicTuple*> elements;
            void add(IDynmaicTuple* element) {
                element->positionInVector = elements.size();
                element->positionInTemplateParameterPack = I;
                elements.push_back(element);
            }
        };
        template <size_t I, class... D>
        struct DynamicTupleHolder {

        };
        template <size_t I, class DH, class... DT>
        struct DynamicTupleHolder<I, DH, DT...> : DynamicTupleLeaf<I, DH>, DynamicTupleHolder<I+1, DT...> {
            using BaseLeaf = DynamicTupleLeaf<I, DH>;
            using BaseDT = DynamicTupleHolder<I+1, DT...>;
            void add(DH* head, size_t N) {
                if (I == N) {
                    BaseLeaf::add(head);
                } else {
                    BaseDT::add(head, N);
                }
            }
            template <class H, class... T>
            void copy(DynamicTupleHolder<I, H, T...>& holder) {
                entities = holder.entities;
                BaseDT::copy(holder);
            }
        };
        template <size_t I, class DH, class... DT>
        struct DynamicTupleHolder<I, DH> : DynamicTupleLeaf<I, DH>, DynamicTupleHolder<I+1> {
            using BaseLeaf = DynamicTupleLeaf<I, DH>;
            using BaseDT = DynamicTupleHolder<I+1>;
            void add(DH* head, size_t N) {
                if (I == N) {
                    BaseLeaf::add(head);
                }
            }
            template <class... T>
            void copy(DynamicTupleHolder<I>& holder) {

            }
        };
        template <size_t I>
        struct DynamicTupleHolder<I> {

        };
        template <typename... TupleTypes>
        struct DynamicTuple {
            DynamicTupleHolder<0, TupleTypes...> contents;
            template <typename H, class = typename std::enable_if_t<contains<H, TupleTypes...>::value>>
            DynamicTyple add (H* head) {
                contents.add(head, index<H, TupleTypes...>());
                return *this;
            }
            template <typename H, class = typename std::enable_if_t<!contains<H, TupleTypes...>::value>>
            DynamicTuple <TupleTypes..., H> add (H* head) {
                DynamicTuple<TupleTypes..., H> tuple;
                tuple.contents.template copy<TupleTypes...>(contents);
                return tuple.add(head);
            }
        };
    }
}
#endif
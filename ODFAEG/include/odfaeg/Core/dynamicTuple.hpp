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
        struct IDynamicTupleElement {
            unsigned int positionInVector, positionInTemplateParameterPack;
            template <size_t I=0>
            static constexpr size_t get(size_t position) {
                if (I == position) {
                    return I;
                } else {
                    get<I+1>();
                }
            }
        };
        template <template <size_t , class...> class T, size_t I, class Head, class... Tail>
        Head* getElement(T<I, Head, Tail...>& tuple, unsigned int index) {
            if (index < tuple.T<I, Head>::elements.size())
                return static_cast<Head*>(tuple.T<I, Head>::elements[index]);
            return nullptr;
        }
        template <size_t I, class D>
        struct DynamicTupleLeaf {
            std::vector<IDynamicTupleElement*> elements;
            void add(IDynamicTupleElement* element) {
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
            void add(IDynamicTupleElement* head, size_t N) {
                if (I == N) {
                    BaseLeaf::add(head);
                } else {
                    BaseDT::add(head, N);
                }
            }
            template <class H, class... T>
            void copy(DynamicTupleHolder<I, H, T...>& holder) {
                BaseLeaf::elements = holder.DynamicTupleLeaf<I, H>::elements;
                BaseDT::copy(holder);
            }
        };
        template <size_t I, class DH>
        struct DynamicTupleHolder<I, DH> : DynamicTupleLeaf<I, DH>, DynamicTupleHolder<I+1> {
            using BaseLeaf = DynamicTupleLeaf<I, DH>;
            using BaseDT = DynamicTupleHolder<I+1>;
            void add(IDynamicTupleElement* head, size_t N) {
                if (I == N) {
                    BaseLeaf::add(head);
                }
            }
            template <class... T>
            void copy(DynamicTupleHolder<I>& holder) {

            }
        };
        template <size_t I>
        struct DynamicTupleHolder<I> : DynamicTupleLeaf<I, IDynamicTupleElement> {
            void add(DH* head, size_t N) {

            }
        };
        template <typename... TupleTypes>
        struct DynamicTuple {
            DynamicTupleHolder<0, TupleTypes...> contents;
            using types = typename std::tuple<TupleTypes...>;
            template <typename H, class = typename std::enable_if_t<contains<H, TupleTypes...>::value>>
            DynamicTuple add (H* head) {
                contents.add(head, index<H, TupleTypes...>());
                return *this;
            }
            template <typename H, class = typename std::enable_if_t<!contains<H, TupleTypes...>::value>>
            DynamicTuple <TupleTypes..., H> add (H* head) {
                DynamicTuple<TupleTypes..., H> tuple;
                tuple.contents.template copy<TupleTypes...>(contents);
                return tuple.add(head);
            }
            template <typename H, class = typename std::enable_if_t<!contains<H, TupleTypes...>::value>>
            DynamicTuple <TupleTypes..., H> addType () {
                DynamicTuple<TupleTypes..., H> tuple;
                tuple.contents.template copy<TupleTypes...>(contents);
                return tuple;
            }
            static constexpr size_t nbTypes() {
                return std::tuple_size<types>::value;
            }
            template <typename T>
            static constexpr size_t getIndexOfTypeT() {
                return index<T, TupleTypes...>();
            }
            template <typename T>
            T* get(unsigned int containerIdx) {
                constexpr size_t I = getIndexOfTypeT<T>();
                return getElement<DynamicTupleLeaf, I>(contents, containerIdx);
            }
            template <size_t I>
            auto get(unsigned int containerIdx) {
                return getElement<DynamicTupleLeaf, I>(contents, containerIdx);
            }
            auto get(unsigned int positionInTemplateParameterPack, unsigned int containerIndex) {
                return get<IDynamicTupleElement::get(positionInTemplateParameterPack)>(containerIndex);
            }
        };
    }
}
#endif

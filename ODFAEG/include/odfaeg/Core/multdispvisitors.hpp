#ifndef ODFAEG_MULTIDISPATCH_VISITOR_HPP
#define ODFAEG_MULTIDISPATCH_VISITOR_HPP

namespace odfaeg
{
    namespace core {


        //Utilitaire pour ajouter un type à la fin d'un tuple
        template<class,class>
        struct cat_type;

        template<class T, class... Arg>
        struct cat_type<std::tuple<Arg...>,T>
        {
            using type =std::tuple<Arg...,T>;
        };


        //Classe qui surcharge accept pour les types concret
        template<class Abstract, class>
        struct acceptable : Abstract
        {
            void accept(const typename Abstract::visitor_type& v)
            {
                v.visit(*this);
            }
        };


        //Classe de base des visiteurs
        //La visite se fait sur le type acceptable qui contient l'information sur le type réel
        template<class, class...>
        struct visitor;

        //Récursion
        template<class Abstract, class Concrete, class... Concrete_Tail>
        struct visitor<Abstract,Concrete,Concrete_Tail...>
            : visitor<Abstract,Concrete_Tail...>
        {
            using visitor<Abstract,Concrete_Tail...>::visit;
            virtual void visit(acceptable<Abstract,Concrete>&) const =0;
        };

        //Condition d'arrêt
        template<class Abstract, class Concrete>
        struct visitor<Abstract,Concrete>
        {
            visitor() =default;
            visitor(const visitor&) =delete;

            virtual ~visitor()
            { }

            visitor& operator=(const visitor&) =delete;

            virtual void visit(acceptable<Abstract,Concrete>&) const =0;
        };


        //Utilitaire pour récupérer le type accepté par la hiérarchie depuis le visiteur
        template<class,class>
        struct accept_type;

        template<
            class Abstract, class... Concrete,
            class T
        >
        struct accept_type<visitor<Abstract,Concrete...>,T>
        {
            using type =acceptable<Abstract,T>;
        };


        //Utilitaire pour récupérer les types concrets de la hiérarchie depuis le visiteur
        template<class>
        struct concrete_type;

        template<class Abstract, class... Concrete>
        struct concrete_type<visitor<Abstract,Concrete...>>
        {
            using type =std::tuple<Concrete...>;
        };


        //Classe de base qui ajoute accept à la classe de base de la hiérarchie
        template<class Abstract, class... Concrete>
        struct accept_visitor
        {
            using visitor_type =visitor<Abstract,Concrete...>;

            accept_visitor() =default;
            accept_visitor(const accept_visitor&) =delete;

            virtual ~accept_visitor()
            { }

            accept_visitor& operator=(const accept_visitor&) =delete;

            virtual void accept(const visitor_type&) =0;
        };


        //Classe dispatcher : c'est un visiteur et un foncteur

        //Condition d'arrêt sur les types de la hiérarchie
        template<
            class To_Visit, class, bool,
            class Visitor, class,
            class Fun
        >
        struct dispatcher : Visitor
        {
            dispatcher(To_Visit& tv, const Fun& f)
                : to_visit(tv), fun(f)
            { }

        protected:
            To_Visit& to_visit;
            Fun fun;
        };

        //Récursion sur les types de la hiérarchie
        template<
            class To_Visit, class Visited,
            class Visitor, class T, class... Concrete,
            class Fun
        >
        struct dispatcher<
            To_Visit,Visited,false,
            Visitor,std::tuple<T,Concrete...>,
            Fun
        >
            : dispatcher<
                To_Visit,Visited,false,
                Visitor,std::tuple<Concrete...>,
                Fun
            >
        {
        private:
            using base =dispatcher<
                To_Visit,Visited,false,
                Visitor,std::tuple<Concrete...>,
                Fun
            >;

        public:
            using base::base;

            //Déclenche la visite sur le bon paramètre
            void operator()() const
            {
                std::get
                    <std::tuple_size<Visited>::value>
                    (base::to_visit)
                .accept(*this);
            }

            using base::visit;
            void visit(typename accept_type<Visitor,T>::type&) const
            {
                using new_visited =typename cat_type<Visited,T>::type;

                //Récursion sur les arguments d'appel
                dispatcher<
                    To_Visit,new_visited,
                    std::tuple_size<To_Visit>::value == std::tuple_size<new_visited>::value,
                    Visitor,typename concrete_type<Visitor>::type,
                    Fun
                >(base::to_visit,base::fun)();
            }
        };

        //Condition d'arrêt sur les paramètre d'appel
        template<
            class To_Visit, class... Visited,
            class Visitor, class Concrete,
            class Fun
        >
        struct dispatcher<
            To_Visit,std::tuple<Visited...>,true,
            Visitor,Concrete,
            Fun
        >
        {
            dispatcher(To_Visit& tv, const Fun& f)
                : to_visit(tv), fun(f)
            { }

            //Déclenche l'appel avec les bon types
            void operator()()
            {
                apply(std::make_index_sequence<sizeof...(Visited)>());
            }

        private:
            template<std::size_t... I>
            void apply(std::index_sequence<I...>)
            {
                fun(reinterpret_cast<Visited&>(std::get<I>(to_visit))...);
            }

            To_Visit& to_visit;
            Fun fun;
        };


        //Classe de base pour les foncteurs à dispatcher
        template<class Fun, class Abstract>
        struct dispatchable
        {
            dispatchable() =default;
            dispatchable(const dispatchable&) =default;

            dispatchable& operator=(const dispatchable&) =default;

            template<class... Arg>
            void apply(Arg&&... arg)
            {
                using used_visitor =typename Abstract::visitor_type;

                auto t =std::forward_as_tuple(std::forward<Arg>(arg)...);
                dispatcher<
                    decltype(t),std::tuple<>,false,
                    used_visitor,typename concrete_type<used_visitor>::type,
                    Fun
                >(t,static_cast<Fun&>(*this))();
            }

        protected:
            ~dispatchable()
            { }
        };
    }
}
#endif

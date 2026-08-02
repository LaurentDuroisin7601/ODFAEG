#ifndef ODFAEG_ANY_HPP
#define ODFAEG_ANY_HPP
#include <typeinfo>
#include <stdexcept>
#include <string>
namespace odfaeg {
	namespace core {
		class Value_base {
		public:
			/** \fn value_case* clone() const = 0;
			*   \brief clone the value.
			*   \return value_base : a pointer to the cloned object.
			*/
			virtual Value_base* clone() const = 0;
			virtual const std::type_info& type() const = 0;
			virtual ~Value_base() {}
		};
		template <typename T>
		class Value : public Value_base {
		public:
			friend class Any;			
			Value(const T& value);
			Value_base* clone() const;
			const std::type_info& type() const;
		private:
			T value;
		};
		class Any {
		public:	
			Any();
			Any(Any const& other);
			Any(Any&& other);
			template <typename T>
			Any(const T& v);
			template <typename T>
			T& get() const;
			template <typename T>
			void set(T v);
			Any& operator=(const Any& other);
			Any& operator=(Any&& other);
			void swap(Any& other);
			const std::type_info& type();
			~Any();
		private:
			Value_base* value;
		};
	}
}
#include "any.inl"
#endif
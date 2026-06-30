module;

#include <typeinfo>
#include <stdexcept>
#include <string>
export module odfaeg.core.any;

export namespace odfaeg {
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
			Value(const T& value) : value(value) {

			}			
			Value_base* clone() const {
				return new Value(value);
			}			
			const std::type_info& type() const {
				return typeid(T);
			}	
		private:
			T value;
		};
		class Any {
		public:	
			Any() : value(nullptr) {}
			Any(Any const& other) : value(other.value ? other.value->clone() : 0) {}
			Any(Any&& other) : value(std::move(other.value)) {}
			template <typename T>
			Any(const T& v) {
				value = new Value(v);
			}	
			template <typename T>
			T& get() const {
				Value<T>* v = dynamic_cast<Value<T>*> (value);
				if (v == 0) {
					throw std::runtime_error(std::string("Any : bad cast exception, type is : ") + value->type().name() + " but given type is : " + typeid(T).name() + "!");
				}
				else {
					return v->t;
				}
			}
			template <typename T>
			void set(T v) {
				delete value;
				value = new Value<T>(v);
			}
			Any& operator=(const Any& other)
			{
				if (&other != this)
				{
					Any copy(other);
					swap(copy);
				}
				return *this;
			}
			Any& operator=(Any&& other)
			{				
				if (&other != this)
				{
					delete value;
					value = std::move(other.value);
					other.value = nullptr;
				}
				return *this;
			}
			void swap(Any& other)
			{
				std::swap(value, other.value);
			}
			const std::type_info& type() {
				return value->type();
			}
			~Any() { delete value; }
		private:
			Value_base* value;
		};
	}
}
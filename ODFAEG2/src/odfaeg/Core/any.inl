namespace odfaeg {
    namespace core {
		template <typename T>	
        Value<T>::Value(const T& value) : value(value) {

        }
        template <typename T>			
        Value_base* Value<T>::clone() const {
            return new Value(value);
        }
        template <typename T>			
        const std::type_info& Value<T>::type() const {
            return typeid(T);
        }	
		Any::Any() : value(nullptr) {}
		Any::Any(Any const& other) : value(other.value ? other.value->clone() : 0) {}
		Any::Any(Any&& other) : value(std::move(other.value)) {}			
        template <typename T>
		Any::Any(const T& v) {
            value = new Value(v);
        }	
		template <typename T>
		T& Any::get() const {
            Value<T>* v = dynamic_cast<Value<T>*> (value);
            if (v == 0) {
                throw std::runtime_error(std::string("Any : bad cast exception, type is : ") + value->type().name() + " but given type is : " + typeid(T).name() + "!");
            }
            else {
                return v->t;
            }
        }
		template <typename T>
		void Any::set(T v) {
		    delete value;
			value = new Value<T>(v);
		}
		Any& Any::operator=(const Any& other)
        {
            if (&other != this)
            {
                Any copy(other);
                swap(copy);
            }
            return *this;
        }
        Any& Any::operator=(Any&& other)
        {				
            if (&other != this)
            {
                delete value;
                value = std::move(other.value);
                other.value = nullptr;
            }
            return *this;
        }
        void Any::swap(Any& other)
        {
            std::swap(value, other.value);
        }
        const std::type_info& Any::type() {
            return value->type();
        }
        Any::~Any() { delete value; }	
    }	
}


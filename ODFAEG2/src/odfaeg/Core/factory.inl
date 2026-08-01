namespace odfaeg {
    namespace core {       
        /**\class Allocator
        *  \brief this struct allocate an object of a derived type and return a pointer of the base type.
        *  \param B : the base type.
        */
        template <typename B>        
        template <typename D, class... Args>
        B*  Allocator<B>::allocate(D* d, Args&&... args) {
            return new D(std::forward<Args>(args)...);
        }  
        template <typename B>
        void BaseFactory<B>::register_type(std::string typeName, FastDelegate<B*> allocatorDelegate) {
            typename std::map<std::string, FastDelegate<B*>>::iterator it = types.find(typeName);
            if (it == types.end()) {
                types[typeName] = allocatorDelegate;
            }
        } 
        template <typename B>
        void BaseFactory<B>::register_function(std::string typeName, std::string funcName, std::string funcArgs, FastDelegate<void> delegate) {
            typename std::map<std::string, FastDelegate<void>>::iterator it = functions.find(typeName + funcName + funcArgs);
            if (it == functions.end()) {
                
                functions[typeName + funcName + funcArgs] = delegate;
            }
        }  
        template <typename B>
        template <typename... A>
        void BaseFactory<B>::callFunction(std::string typeName, std::string funcName, std::string funcArgs, A&&... args) {
            typename std::map<std::string, FastDelegate<void>>::iterator it = functions.find(typeName + funcName + funcArgs);
            
            if (it != functions.end()) {
                it->second.bind(std::forward<A>(args)...);
                (it->second)();
            }
            else {
                throw std::runtime_error("Unregistred function exception!");
            }
        }  
        template <typename B>
        template <class... Args>
        B* BaseFactory<B>::create(std::string typeName, Args&&... args) {
            typename std::map<std::string, FastDelegate<B*>>::iterator it = types.find(typeName);
            if (it != types.end()) {                    
                return (it->second)();
            }
            throw std::runtime_error("Unregistred type exception!" + typeName);
        }
        template <typename B> 
        std::string BaseFactory<B>::getTypeName(B* type) {
            typename std::map<std::string, FastDelegate<B*>>::iterator it = types.find(typeid(*type).name());
            if (it != types.end())
                return it->first;
            return "";
        }
    }
}
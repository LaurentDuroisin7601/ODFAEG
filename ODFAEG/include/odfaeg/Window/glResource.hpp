#ifndef ODFAEG_GL_RESOURCE_HPP
#define ODFAEG_GL_RESOURCE_HPP
#include "export.hpp"
namespace odfaeg {
    namespace window {
        //typedef void(*ContextDestroyCallback)(void*);
        class ODFAEG_WINDOW_API GLResource {
            public :
            GLResource();
            ~GLResource();
             /*static void registerContextDestroyCallback(ContextDestroyCallback callback, void* arg);
            ////////////////////////////////////////////////////////////
            /// \brief RAII helper class to temporarily lock an available context for use
            ///
            ////////////////////////////////////////////////////////////
            class TransientContextLock : sf::NonCopyable
            {
            public:
                ////////////////////////////////////////////////////////////
                /// \brief Default constructor
                ///
                ////////////////////////////////////////////////////////////
                TransientContextLock();

                ////////////////////////////////////////////////////////////
                /// \brief Destructor
                ///
                ////////////////////////////////////////////////////////////
                ~TransientContextLock();
            };*/
        };
    }
}
#endif

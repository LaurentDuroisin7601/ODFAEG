#include "../../../include/odfaeg/Window/glResource.hpp"
#include "../../../include/odfaeg/Window/contextImpl.hpp"
#include <iostream>
namespace odfaeg {
    namespace window {
        GLResource::GLResource() {
            ContextImpl::initResource();
        }
        GLResource::~GLResource() {
            ContextImpl::cleanupResource();
        }
        ////////////////////////////////////////////////////////////
        /*void GLResource::registerContextDestroyCallback(ContextDestroyCallback callback, void* arg)
        {
            ContextImpl::registerContextDestroyCallback(callback, arg);
        }
        ////////////////////////////////////////////////////////////
        GLResource::TransientContextLock::TransientContextLock()
        {
            ContextImpl::acquireTransientContext();
        }


        ////////////////////////////////////////////////////////////
        GLResource::TransientContextLock::~TransientContextLock()
        {
            ContextImpl::releaseTransientContext();
        }*/
    }
}


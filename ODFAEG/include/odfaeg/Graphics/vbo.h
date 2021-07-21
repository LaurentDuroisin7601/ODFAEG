#ifndef ODFAEG_VBO_HPP
#define ODFAEG_VBO_HPP
#include <vector>
#include "export.hpp"
namespace odfaeg {
    namespace graphic {
        class ODFAEG_GRAPHICS_API VBO {
        friend class VertexArray;
        public:
            VBO();
            void create (const void* data, const unsigned int dataSize);
            bool update (const void* data, const unsigned int dataSize);
            void bind();
            void unbind();
            static bool isAvailable();
            virtual ~VBO();
        private :
            unsigned int m_vbo;
        };
    }
}
#endif // ODFAEG_VBO_HPP

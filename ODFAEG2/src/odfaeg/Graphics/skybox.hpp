#include "vertexBuffer.hpp"
#include "texture.hpp"
namespace odfaeg {
    namespace graphic {
        Skybox(float size, std::vector<std::string> filepath);
        VertexBuffer& getVertexBuffer();
        Texture& getTexture();
    }
}
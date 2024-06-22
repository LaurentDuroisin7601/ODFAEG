#include "application.h"
#include "odfaeg/Math/triangle.h"
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::core;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
int main (int argv, char* argc[]) {
    //Window window(sf::VideoMode(800, 600), "test", sf::Style::Default, ContextSettings(0, 0, 4, 3, 0));
    /*Texture tex;
    Window window(sf::VideoMode(800, 600), "test", sf::Style::Default, ContextSettings(0, 0, 4, 3, 0));
    glewInit();
    tex.loadFromFile("tilesets/mesh_puddingmill/puddingmill_tex.png");*/

    /*sf::Image image;
    image.loadFromFile("tilesets/mesh_puddingmill/puddingmill_tex.png");
    GLuint texture;
    glCheck(glGenTextures(1, &texture));
    glCheck(glBindTexture(GL_TEXTURE_2D, texture));
    glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.getSize().x, image.getSize().y, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr()));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));*/
    //std::vector<sf::Uint8> pixels(image.getSize().x * image.getSize().y * 4);*/


    // Texture is not padded nor flipped, we can use a direct copy
    /*glCheck(glBindTexture(GL_TEXTURE_2D, texture));
    glCheck(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]));*/

    /*for (unsigned int i = 0; i < image.getSize().x * image.getSize().y * 4; i++)
        std::cout<<"pixel : "<<(int) pixels[i]<<std::endl;*/
    /*sf::Image img = tex.copyToImage();
    for (unsigned int x = 0; x < img.getSize().x; x++) {
        for (unsigned int y = 0; y < img.getSize().y; y++) {
            sf::Color pixel = img.getPixel(x, y);
            std::cout<<"pixel : "<<(int) pixel.r<<","<<(int) pixel.g<<","<<(int) pixel.b<<std::endl;
        }
    }*/
    /*Triangle triangle(Vec3f(107, 270, 0.999918), Vec3f(140, 279, 0.999918), Vec3f(114, 279, 0.9999));

    Ray ray(Vec3f(120, 277, 0), Vec3f(120, 277, 1000));
    Vec3f i1, i2;
    if (triangle.intersectsWhere(ray, i1, i2))
        std::cout<<i1<<std::endl;
    while (true);*/

    MyAppli appli(Vec2f(800, 600), "test");
    return appli.exec();
}

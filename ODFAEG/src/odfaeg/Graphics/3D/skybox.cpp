#include "../../../../include/odfaeg/Graphics/3D/skybox.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {

            #ifdef VULKAN
            Skybox::Skybox(std::vector<std::string> filepaths, EntityFactory& factory, window::Device& vkDevice) : GameObject(math::Vec3f(0, 0, 0),math::Vec3f(2, 2, 2),math::Vec3f(1,1,1),"E_SKYBOX", factory), vkDevice(vkDevice), skyboxCM(vkDevice) {
                skyboxCM.loadCubeMapFromFile(filepaths);
                VertexArray va1(Quads, 4, this);
                Vertex v1(math::Vec3f(1, -1, -1));
                Vertex v2(math::Vec3f(1, 1, -1));
                Vertex v3(math::Vec3f(1, 1, 1));
                Vertex v4(math::Vec3f(1, -1, 1));
                va1[0] = v1;
                va1[1] = v2;
                va1[2] = v3;
                va1[3] = v4;
                Material material1;
                material1.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face1 (va1, material1, getTransform());
                //Gauche.
                VertexArray va2(Quads, 4, this);
                Vertex v5(math::Vec3f(-1, -1, -1));
                Vertex v6(math::Vec3f(-1, 1, -1));
                Vertex v7(math::Vec3f(-1, 1, 1));
                Vertex v8(math::Vec3f(-1, -1, 1));
                va2[0] = v5;
                va2[1] = v6;
                va2[2] = v7;
                va2[3] = v8;
                Material material2;
                material2.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face2 (va2, material2, getTransform());
                //Dessus
                VertexArray va3(Quads, 4, this);
                Vertex v9(math::Vec3f(-1, 1, -1));
                Vertex v10(math::Vec3f(1, 1, -1));
                Vertex v11(math::Vec3f(1, 1, 1));
                Vertex v12(math::Vec3f(-1, 1, 1));
                va3[0] = v9;
                va3[1] = v10;
                va3[2] = v11;
                va3[3] = v12;
                Material material3;
                material3.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face3 (va3, material3, getTransform());
                //Dessous.
                VertexArray va4(Quads, 4, this);
                Vertex v13(math::Vec3f(-1, -1, -1));
                Vertex v14(math::Vec3f(1, -1, -1));
                Vertex v15(math::Vec3f(1, -1, 1));
                Vertex v16(math::Vec3f(-1, -1, 1));
                va4[0] = v13;
                va4[1] = v14;
                va4[2] = v15;
                va4[3] = v16;
                Material material4;
                material4.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face4 (va4, material4, getTransform());
                /*for (unsigned int i = 0; i < face4.getVertexArray().getVertexCount(); i++) {
                    std::cout<<"vertex position : "<<face4.getVertexArray()[i].position.x<<std::endl;
                }*/
                //Devant
                VertexArray va5(Quads, 4, this);
                Vertex v17(math::Vec3f(-1, -1, 1));
                Vertex v18(math::Vec3f(1, -1, 1));
                Vertex v19(math::Vec3f(1, 1, 1));
                Vertex v20(math::Vec3f(-1, 1, 1));
                va5[0] = v17;
                va5[1] = v18;
                va5[2] = v19;
                va5[3] = v20;
                Material material5;
                material5.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face5 (va5, material5, getTransform());

                //Derrière.
                VertexArray va6(Quads, 4, this);
                Vertex v21(math::Vec3f(-1, -1, -1));
                Vertex v22(math::Vec3f(1, -1, -1));
                Vertex v23(math::Vec3f(1, 1, -1));
                Vertex v24(math::Vec3f(-1, 1, -1));
                va6[0] = v21;
                va6[1] = v22;
                va6[2] = v23;
                va6[3] = v24;
                Material material6;
                material6.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face6 (va6, material6, getTransform());

                addFace(face1);
                addFace(face2);
                addFace(face3);
                addFace(face4);
                addFace(face5);
                addFace(face6);
            }
            void Skybox::onDraw(RenderTarget& target, RenderStates states) {
                states.texture = &skyboxCM;
                for (unsigned int i = 0; i < getFaces().size(); i++) {
                    target.draw (getFace(i)->getVertexArray(),states);
                }
            }
            Texture& Skybox::getTexture() {
                return skyboxCM;
            }
            Entity* Skybox::clone() {
                Skybox* skybox = factory.make_entity<Skybox>(filepaths,factory, vkDevice);
                GameObject::copy(skybox);
                skybox->skyboxCM = getTexture();
                return skybox;
            }
            #else
            Skybox::Skybox(std::vector<sf::Image> skyboxImages, EntityFactory& factory) : GameObject(math::Vec3f(0, 0, 0),math::Vec3f(2, 2, 2),math::Vec3f(1,1,1),"E_SKYBOX", factory) {
                skyboxCM.createCubeMap(skyboxImages[0].getSize().x, skyboxImages[0].getSize().y, skyboxImages);
                //Droite.
                VertexArray va1(Quads, 4, this);
                Vertex v1(math::Vec3f(1, -1, -1));
                Vertex v2(math::Vec3f(1, 1, -1));
                Vertex v3(math::Vec3f(1, 1, 1));
                Vertex v4(math::Vec3f(1, -1, 1));
                va1[0] = v1;
                va1[1] = v2;
                va1[2] = v3;
                va1[3] = v4;
                Material material1;
                material1.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face1 (va1, material1, getTransform());
                //Gauche.
                VertexArray va2(Quads, 4, this);
                Vertex v5(math::Vec3f(-1, -1, -1));
                Vertex v6(math::Vec3f(-1, 1, -1));
                Vertex v7(math::Vec3f(-1, 1, 1));
                Vertex v8(math::Vec3f(-1, -1, 1));
                va2[0] = v5;
                va2[1] = v6;
                va2[2] = v7;
                va2[3] = v8;
                Material material2;
                material2.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face2 (va2, material2, getTransform());
                //Dessus
                VertexArray va3(Quads, 4, this);
                Vertex v9(math::Vec3f(-1, 1, -1));
                Vertex v10(math::Vec3f(1, 1, -1));
                Vertex v11(math::Vec3f(1, 1, 1));
                Vertex v12(math::Vec3f(-1, 1, 1));
                va3[0] = v9;
                va3[1] = v10;
                va3[2] = v11;
                va3[3] = v12;
                Material material3;
                material3.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face3 (va3, material3, getTransform());
                //Dessous.
                VertexArray va4(Quads, 4, this);
                Vertex v13(math::Vec3f(-1, -1, -1));
                Vertex v14(math::Vec3f(1, -1, -1));
                Vertex v15(math::Vec3f(1, -1, 1));
                Vertex v16(math::Vec3f(-1, -1, 1));
                va4[0] = v13;
                va4[1] = v14;
                va4[2] = v15;
                va4[3] = v16;
                Material material4;
                material4.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face4 (va4, material4, getTransform());
                /*for (unsigned int i = 0; i < face4.getVertexArray().getVertexCount(); i++) {
                    std::cout<<"vertex position : "<<face4.getVertexArray()[i].position.x<<std::endl;
                }*/
                //Devant
                VertexArray va5(Quads, 4, this);
                Vertex v17(math::Vec3f(-1, -1, 1));
                Vertex v18(math::Vec3f(1, -1, 1));
                Vertex v19(math::Vec3f(1, 1, 1));
                Vertex v20(math::Vec3f(-1, 1, 1));
                va5[0] = v17;
                va5[1] = v18;
                va5[2] = v19;
                va5[3] = v20;
                Material material5;
                material5.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face5 (va5, material5, getTransform());

                //Derrière.
                VertexArray va6(Quads, 4, this);
                Vertex v21(math::Vec3f(-1, -1, -1));
                Vertex v22(math::Vec3f(1, -1, -1));
                Vertex v23(math::Vec3f(1, 1, -1));
                Vertex v24(math::Vec3f(-1, 1, -1));
                va6[0] = v21;
                va6[1] = v22;
                va6[2] = v23;
                va6[3] = v24;
                Material material6;
                material6.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face6 (va6, material6, getTransform());

                addFace(face1);
                addFace(face2);
                addFace(face3);
                addFace(face4);
                addFace(face5);
                addFace(face6);

                for (unsigned int i = 0; i < 6; i++) {
                    getFaces()[i].getVertexArray().computeNormals();
                }

                skyboxImgs = skyboxImages;
            }
            void Skybox::onDraw(RenderTarget& target, RenderStates states) {
                for (unsigned int i = 0; i < getFaces().size(); i++) {
                    states.texture = const_cast<Skybox*>(this)->getFace(i)->getMaterial().getTexture();
                    target.draw (getFace(i)->getVertexArray(),states);
                }
            }
            Texture& Skybox::getTexture() {
                return skyboxCM;
            }
            Entity* Skybox::clone() {
                Skybox* skybox = factory.make_entity<Skybox>(skyboxImgs,factory);
                GameObject::copy(skybox);
                skybox->skyboxCM = skyboxCM;
                return skybox;
            }
            #endif
        }
    }
}

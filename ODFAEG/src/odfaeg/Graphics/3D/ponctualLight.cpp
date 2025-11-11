#include "../../../../include/odfaeg/Graphics/3D/ponctualLight.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            using namespace std;

            //Crée une lumière avec sa position, son intensité et son type.
            PonctualLight::PonctualLight (math::Vec3f center, float r1, float r2, float r3, float intensity, Color color, int quality, EntityFactory& factory, Entity *parent) :
                EntityLight (center, color, r1, r2, r3, height, "E_PONCTUAL_LIGHT", factory, "", parent) {
                this->littleRadius = r1;
                this->bigRadius = r2;
                this->quality = quality;
                this->intensity = intensity;
                this->color.r = color.r * intensity / 255;
                this->color.g = color.g * intensity / 255;
                this->color.b = color.b * intensity / 255;
                initTriangles ();
            }
            Entity* PonctualLight::clone() {
                PonctualLight* pl = factory.make_entity<PonctualLight>(factory);
                GameObject::copy(pl);
                pl->intensity = intensity;
                pl->color = color;
                pl->height = height;
                pl->triangles = triangles;
                pl->quality = quality;
                return pl;
            }
            void PonctualLight::initTriangles() {
                for (unsigned int i = 0; i < triangles.size(); i++) {
                    delete triangles[i];
                }
                triangles.clear();
                /*float angle = math::Math::toRadians(360) / quality;

                Color color2 (color.r, color.g, color.b, 0);
                int end = quality;

                for (int i = 0; i < end; i++) {
                    math::Vec3f v1, v2, v3;
                    v1 = math::Vec3f (0, 0, 0);
                    v2 = math::Vec3f (getSize().x() * 0.5f * math::Math::cosinus (i * angle), getSize().y() * 0.5f * math::Math::sinus (i * angle), 0);
                    v3 = math::Vec3f (getSize().x() * 0.5f * math::Math::cosinus ((i + 1) * angle), getSize().y() * 0.5f * math::Math::sinus ((i + 1) * angle), 0);
                    VertexArray *triangle = new VertexArray (Triangles, 3, this);
                    (*triangle)[0] = Vertex(v1, color);
                    (*triangle)[1] = Vertex(v2, color);
                    (*triangle)[2] = Vertex(v3, color);
                    addTriangle(triangle);
                }
                for (int i = 0; i < end; i++) {
                    math::Vec3f v1, v2, v3;
                    v1 = math::Vec3f (0, 0, 0);
                    v2 = math::Vec3f (getSize().x() * 0.5f * math::Math::cosinus (i * angle), 0, getSize().z() * 0.5f * math::Math::sinus (i * angle));
                    v3 = math::Vec3f (getSize().x() * 0.5f * math::Math::cosinus ((i + 1) * angle), 0, getSize().z() * 0.5f * math::Math::sinus ((i + 1) * angle));
                    VertexArray *triangle = new VertexArray (Triangles, 3, this);
                    (*triangle)[0] = Vertex(v1, color);
                    (*triangle)[1] = Vertex(v2, color);
                    (*triangle)[2] = Vertex(v3, color);
                    addTriangle(triangle);
                }*/

                const int stacks = quality, slices = quality;
                const int vertexCount = (stacks + 1) * (slices + 1);
                VertexArray* vertices = new VertexArray(Triangles, vertexCount, this);

                int n = 0;
                for (uint32_t i = 0; i <= stacks; ++i) {
                    float phi = PI * i / stacks;
                    float y = getSize().x() * 0.5f * math::Math::cosinus(phi);
                    float r = getSize().x() * 0.5f * math::Math::sinus(phi);

                    for (uint32_t j = 0; j <= slices; ++j) {
                        float theta = 2 * PI * j / slices;
                        float x = r * math::Math::cosinus(theta);
                        float z = r * math::Math::sinus(theta);

                        math::Vec3f pos(x, y, z);
                        math::Vec3f normal = pos.normalize();
                        math::Vec2f uv((float)j / slices, (float)i / stacks);
                        Vertex vertex(pos, color, uv);
                        vertex.normal = normal;
                        (*vertices)[n] = vertex;
                        n++;
                    }
                }


                for (uint32_t i = 0; i < stacks; ++i) {
                    for (uint32_t j = 0; j < slices; ++j) {
                        uint32_t first = i * (slices + 1) + j;
                        uint32_t second = first + slices + 1;

                        vertices->addIndex(first);
                        vertices->addIndex(second);
                        vertices->addIndex(first + 1);

                        vertices->addIndex(second);
                        vertices->addIndex(second + 1);
                        vertices->addIndex(first + 1);
                    }
                }

                addTriangle(vertices);
            }
            //Ajoute un triangle à la source lumineuse.
            void PonctualLight::addTriangle (VertexArray *triangle) {
                //A faire sur la render texture.
                //triangle->SetBlendMode(Blend::Add);
                //Plus nécessaire avec la ODFAEG 2.0.
                /*triangle->EnableFill(true);
                triangle->EnableOutline(false);*/
                Material material;
                math::Vec4f center = getCenter() - getSize()*0.5f;
                //std::cout<<"radius : "<<getSize().x()<<std::endl;
                center[3] = getSize().x() * 0.5f;
                material.setLightInfos(center,getColor());
                Face face (*triangle,material,getTransform());
                addFace(face);
                triangles.push_back(triangle);
            }
            int PonctualLight::getIntensity() {
                return intensity;
            }
            vector<VertexArray*> PonctualLight::getTris () {
                return triangles;
            }
            void PonctualLight::onDraw(RenderTarget &target, RenderStates states) {
                for (unsigned int i = 0; i < triangles.size(); i++) {
                     states.transform = getTransform();
                     target.draw(*triangles[i], states);
                }
            }

            bool PonctualLight::operator== (Entity &other) {
                   if (!GameObject::operator==(other))
                       return false;
                   return color == other.getColor() &&
                       intensity == other.getIntensity();
            }
            PonctualLight::~PonctualLight () {
                for (unsigned int i = 0; i < triangles.size(); i++) {
                    delete triangles[i];
                }
                triangles.clear();

            }
        }
    }
}

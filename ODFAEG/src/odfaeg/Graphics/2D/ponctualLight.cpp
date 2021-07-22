#include "../../../../include/odfaeg/Graphics/2D/ponctualLight.h"

#include "../../../../include/odfaeg/Math/transformMatrix.h"
#include "../../../../include/odfaeg/Physics/boundingPolyhedron.h"
namespace odfaeg {
    namespace graphic {
        namespace g2d {
            /*Crée une lumière de position poistion, suivant la direction dir, de rayon radius,
             et d'intensité intensity, de type Type*/
            using namespace std;
            using namespace sf;
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

                this->intensity = intensity;
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
            //On initialise les triangles lumineux pour former une élipse.
            void PonctualLight::initTriangles () {

                for (unsigned int i = 0; i < triangles.size(); i++) {
                    delete triangles[i];
                }
                triangles.clear();
                float angle = math::Math::toRadians(360) / quality;

                Color color2 (color.r, color.g, color.b, 0);
                int end = quality;

                for (int i = 0; i < end; i++) {
                    Vector3f v1, v2, v3;
                    v1 = Vector3f (0, 0, 0);
                    v2 = Vector3f (getSize().x * 0.5f * math::Math::cosinus (i * angle), getSize().y * 0.5f * math::Math::sinus (i * angle), 0);
                    v3 = Vector3f (getSize().x * 0.5f * math::Math::cosinus ((i + 1) * angle), getSize().y * 0.5f * math::Math::sinus ((i + 1) * angle), 0);
                    VertexArray *triangle = new VertexArray (Triangles, 3, this);
                    (*triangle)[0] = Vertex(v1, color);
                    (*triangle)[1] = Vertex(v2, color);
                    (*triangle)[2] = Vertex(v3, color);
                    addTriangle(triangle);
                }
            }
            //Ajoute un triangle à la source lumineuse.
            void PonctualLight::addTriangle (VertexArray *triangle) {
                //A faire sur la render texture.
                //triangle->SetBlendMode(Blend::Add);
                //Plus nécessaire avec la SFML 2.0.
                /*triangle->EnableFill(true);
                triangle->EnableOutline(false);*/
                Material material;
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

            int PonctualLight::compAlphaFromPoint (math::Vec3f &point) {
                math::Vec3f v1 = math::Vec3f(point.x, point.y, 0) - math::Vec3f(getCenter().x, getCenter().y, 0);
                math::Vec3f v2 = v1.normalize();
                v2.x *= littleRadius * 0.5f;
                v2.y *= bigRadius * 0.5f;
                float m = v1.magnSquared() / v2.magnSquared();
                float f = 255.f * v2.magnitude();
                return f * m;
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


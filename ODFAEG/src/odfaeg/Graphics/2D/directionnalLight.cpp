
#include "../../../../include/odfaeg/Graphics/2D/directionnalLight.h"
#include "../../../../include/odfaeg/Math/ray.h"
#include "../../../../include/odfaeg/Math/computer.h"
#include "../../../../include/odfaeg/Math/transformMatrix.h"
#include "../../../../include/odfaeg/Physics/boundingPolyhedron.h"
namespace odfaeg {
    namespace graphic {
        namespace g2d {
            /*Crée une lumière de position poistion, suivant la direction dir, de rayon radius,
             et d'intensité intensity, de type Type*/
            using namespace std;
            using namespace sf;
            //Crée une lumière avec sa position, sa direction, sa hauteur, son rayon, son intensité et son typpe.
            DirectionnalLight::DirectionnalLight (math::Vec3f center, math::Vec3f dir, int height, float r1, float r2, float r3, float intensity, Color color, int quality, EntityFactory& factory, Entity *parent) :
                EntityLight (center, color, r1, r2, r3, height, "E_DIRECTIONNAL_LIGHT", factory, "", parent) {

                this->quality = quality;
                this->dir = dir;
                this->intensity = intensity;
                bigRadius = r1;
                littleRadius = r2;
                this->color.r = color.r * intensity / 255;
                this->color.g = color.g * intensity / 255;
                this->color.b = color.b * intensity / 255;
                initTriangles ();
            }
            //On initialise les triangles lumineux pour former une élipse.
            void DirectionnalLight::initTriangles () {

                for (unsigned int i = 0; i < triangles.size(); i++) {
                    delete triangles[i];
                }
                triangles.clear();
                float angle = math::Math::toRadians(360) / quality;

                Color color2 (color.r, color.g, color.b, 0);
                int end = quality;

                for (int i = 0; i < end; i++) {
                    Vector3f v1, v2, v3;
                    math::Vec3f center = getLightCenter();
                    v1 = Vector3f (center.x, center.y, center.z);
                    v2 = Vector3f (center.x + getSize().x * 0.5f * math::Math::cosinus (i * angle), center.y + getSize().y * 0.5f * math::Math::sinus (i * angle), 0);
                    v3 = Vector3f (center.x + getSize().x * 0.5f * math::Math::cosinus ((i + 1) * angle), center.y + getSize().y * 0.5f * math::Math::sinus ((i + 1) * angle), 0);
                    VertexArray *triangle = new VertexArray (Triangles);
                    triangle->append(Vertex(v1, color));
                    triangle->append(Vertex(v2, color2));
                    triangle->append(Vertex(v3, color2));
                    addTriangle(triangle);
                }
                computeCone();
            }
            //Ajoute un triangle à la source lumineuse.
            void DirectionnalLight::addTriangle (VertexArray *triangle) {
                //A faire sur la render texture.
                //triangle->SetBlendMode(Blend::Add);
                //Plus nécessaire avec la SFML 2.0.
                /*triangle->EnableFill(true);
                triangle->EnableOutline(false);*/
                triangles.push_back(triangle);
            }
            math::Vec3f DirectionnalLight::getDir() {

                return dir;
            }
            int DirectionnalLight::getIntensity() {
                return intensity;
            }

            vector<VertexArray*> DirectionnalLight::getTris () {
                return triangles;
            }
            void DirectionnalLight::onDraw(RenderTarget &target, RenderStates states) const {

                for (unsigned int i = 0; i < triangles.size(); i++)
                    target.draw(*triangles[i], states);
                for (unsigned int i = 0; i < coneTris.size(); i++)
                    target.draw (*coneTris[i], states);
            }
            //Recherche l'intersection entre le triangle numéro n de la lumière et le segment s1.
            /*void DirectionnalLight::checkInterTri (int n, Segment &s1) {

                int  i1 = -1, i2 = -1;
                Vec2f pointI1, pointI2;
                Segment s2;
                BoundingPolygon bp;
                //On crée un polygon qui est le triangle lumineux et on cherche si les côtés du triangle intersecte le segment.
                for (int k = 0; k < 3; k++) {

                    Vec2f *a = new Vec2f ((*triangles[n])[k].position.x, (*triangles[n])[k].position.y);
                    Vec2f b;
                    bp.addPoint(a);
                    if (k == 2) {

                        b = Vec2f ((*triangles[n])[0].position.x, (*triangles[n])[0].position.y);

                    } else {

                        b = Vec2f ((*triangles[n])[k + 1].position.x, (*triangles[n])[k + 1].position.y);
                    }

                    s2 = Segment (*a, b);

                    float iseg = s1.intersectsWhere(s2);
                    float iseg2 = s1.intersectWhereOther(s2);
                    //Si les segments sont en intersection, on recherche le(s) point(s) d'intersection. (2 points d'intersection maximum.)
                    if (iseg >= 0.f && iseg <= 1.f && iseg2 >= 0.f && iseg2 <= 1.f) {

                        if (i1 == -1) {

                            i1 = k;
                            pointI1 = s1.getOrig() + s1.getDir() * iseg;
                        } else if (i2 == -1) {

                            i2 = k;
                            pointI2 = s1.getOrig() + s1.getDir() * iseg;
                        }
                    }
                }
                //Si il y a 2 intersection alors le segment intersecte le triangle complètement.
                //Sinon l'extrêmité du segment est dans le triangle.
                if (i1 != -1 && i2 != -1) {
                    //On découpe les triangles.
                    intersectsWallCompletely(pointI1, pointI2, i1, i2, n);
                } else if (i1 != -1 && i2 == -1) {
                    //On recherche quel extrêmité du segment est dans le triangle.
                    Vec2f orig = s1.getOrig();
                    Vec2f ext = s1.getExt();
                    if (bp.isPointInside(orig))
                        pointI2 = orig;
                    else if (bp.isPointInside(ext))
                        pointI2 = ext;
                    //On découpe les triangles.
                    intersectsWallCorner(pointI1, pointI2, n, s1, i1);
                }
            }*/
            /*
            * Modifie ou ajoute des triangles suites aux intersections des triangles de lumières avec le segment du mur.
            * (Plusieurs cas sont à examiner.)
            * pointI1 = point d'intersection 1.
            * pointI2 = point d'intersection 2, si pas de deuxième intersection, il vaut l'extrêmité du segement qui est
            * dans le triangles!
            * i1 et i2 sont les numéros des côtés qui intersectent le segment. (-1 si pas d'intersections.)
            * numTris est le numéro du triangle dans lequel à lieu l'/les intersection(s).
            */
            /*void DirectionnalLight::intersectsWallCompletely(Vec2f &pointI1, Vec2f &pointI2, int i1, int i2, int numTris) {


                * Cas 1 : le segment intersecte les deux côté du triangle. (mais pas le côté extrêmité.)
                * on ne fait que de déplacer deux points extrêmes du triangle au points d'intersections.
                *         | /| |\ |
                *         |/ | | \|
                *         |  | |  |
                *        /|  | |  |\
                *       / |  | |  | \
                *       \ |  | |  | /
                *        \|  | |  |/
                *         |  | |  |
                *         |\ | | /|
                *         | \| |/ |
                */


                /*Color color = (*triangles[numTris])[0].color;
                Color color2 (color.r, color.g, color.b, 0);
                if (i1 == 0 && i2 == 2) {

                    (*triangles[numTris])[1].position = Vector3f(pointI1.x, pointI1.y, 0);
                    int alpha = compAlphaFromPoint(pointI1);
                    color2.a = alpha;
                    (*triangles[numTris])[1].color = color2;

                    (*triangles[numTris])[2].position = Vector3f(pointI2.x, pointI2.y, 0);
                    alpha = compAlphaFromPoint(pointI2);
                    color2.a = alpha;
                    (*triangles[numTris])[2].color = color2;



                *  Cas 2 : le segment intersecte le 1er côté du triangle et le côté extrême.
                *  il faut déplacer le 1 er point au 2ème point d'intersection et rajouté un triangle.
                *
                *      \  /|     \|\
                *       \/ |      | \
                *       /\ |      |\ \
                *       \ \|      | \/
                *        \ |      | /\
                *         \|\     |/  \
                *
                *
                */
                /*} else if (i1 == 0 && i2 == 1) {

                    Vector3f a ((*triangles[numTris])[0].position.x, (*triangles[numTris])[0].position.y, 0);
                    Vector3f b (pointI1.x, pointI1.y, 0);
                    Vector3f c (pointI2.x, pointI2.y, 0);
                    VertexArray *triangle = new VertexArray (Triangles);
                    triangle->append(Vertex(a, color));
                    int alpha = compAlphaFromPoint(pointI1);
                    color2.a = alpha;

                    triangle->append(Vertex(b, color2));
                    alpha = compAlphaFromPoint(pointI2);

                    color2.a = alpha;
                    triangle->append(Vertex(c, color2));
                    addTriangle(triangle);
                    (*triangles[numTris])[1].position = Vector3f(pointI2.x, pointI2.y, 0);
                    (*triangles[numTris])[1].color = color2;


                    *  Cas 3 : le segment intersecte le 3 ème côté du triangle et le côté extrême.
                    *  il faut déplacer le points 2 à la 1 ère intersection et rajouter un triangle.
                    *
                    *         /| /    |\  /
                    *        / |/     | \/
                    *       /  |      | /\
                    *       \ /|      |/ /
                    *        \ |      | /
                    *       / \|     /|/
                    *
                    */

                /*} else if (i1 == 1 && i2 == 2) {
                    Vector3f a = Vector3f((*triangles[numTris])[0].position.x, (*triangles[numTris])[0].position.y, 0);
                    Vector3f b (pointI1.x, pointI1.y, 0);
                    Vector3f c (pointI2.x, pointI2.y, 0);
                    VertexArray *triangle = new VertexArray (Triangles);
                    triangle->append(Vertex(a, color));
                    int alpha1 = compAlphaFromPoint(pointI1);

                    color2.a = alpha1;
                    triangle->append(Vertex(b, color2));
                    int alpha2 = compAlphaFromPoint(pointI2);

                    color2.a = alpha2;
                    triangle->append(Vertex(c, color2));
                    addTriangle(triangle);
                    (*triangles[numTris])[2].position = Vector3f(pointI1.x, pointI1.y, 0);
                    color2.a = alpha1;
                    (*triangles[numTris])[2].color = color2;
                }
            }

            * Modifie ou ajoute des triangles suites aux intersections des triangles de lumières avec une extrêmité du segment du mur.
            * (Plusieurs cas sont à examiner.)
            * pointI1 = point d'intersection 1.
            * pointI2 = point d'intersection 2, si pas de deuxième intersection, il vaut l'extrêmité du segement qui est
            * dans le triangles!
            * i1 et i2 sont les numéros des côtés qui intersectent le segment. (-1 si pas d'intersections.)
            * numTris est le numéro du triangle dans lequel à lieu l'/les intersection(s).
            */
            /*void DirectionnalLight::intersectsWallCorner (Vec2f &pointI1, Vec2f &pointI2, int n, Segment &s1, int i) {

                *  Cas 4  : le segment intersecte 1 seul côté du triangle.
                *  il faut découper le triangle en 2.
                *
                *|\       /|      |\        /|
                *| \     / |      | \      / |
                *|  \   /  |      |  \    /  |
                *| \/   \/ |      |/ /    \ \|
                *| /\   /\ |      | /      \ |
                *|/  \ /  \|     /|/        \|\
                *
                *|\ /   \ /|     \|\        /|/
                *| \     / |      | \      / |
                *|/ \   / \|      |\ \    / /|
                *|  /   \  |      |  /    \  |
                *| /     \ |      | /      \ |
                *|/       \|      |/        \|
                *
                */
                //On prend le vecteur qu'il y a entre le 1er point du triangle et l'extrêmité du segment.
                /*Color color = (*triangles[n])[0].color;
                Color color2 (color.r, color.g, color.b, 0);
                Vec2f a ((*triangles[n])[0].position.x, (*triangles[n])[0].position.y);
                //Vecteur entre le centre de la lumière et le coin du mur.
                Vec2f result = pointI2 - a;
                //Si le coin du mur n'est pas au au centre de la lumière.
                if (!result.isNulVector()) {
                    //Si le côté de l'intersection n'est pas le côté extrême du triangle.
                    if (i != 1) {
                        //Si det > 0 alors l'intersection est du côté du 2ème côté du triangle, sinon, l'intersection est de l'autre côté.
                        if (i == 0) {
                            int alphaP1 = (*triangles[n])[1].color.a;
                            int alphaP2 = (*triangles[n])[2].color.a;
                            int alphaPI1 = compAlphaFromPoint(pointI1);
                            int alphaPI2 = compAlphaFromPoint(pointI2);
                            VertexArray *quad = new VertexArray(Quads);
                            quad->append(Vertex((*triangles[n])[0].position, color));
                            color2.a = alphaPI2;
                            quad->append(Vertex(Vector3f(pointI2.x, pointI2.y, 0), color2));
                            color2.a = alphaP1;
                            quad->append(Vertex((*triangles[n])[1].position, color2));
                            color2.a = alphaP2;
                            quad->append(Vertex((*triangles[n])[2].position, color2));
                            color2.a = alphaPI1;
                            (*triangles[n])[1].position = Vector3f(pointI1.x,pointI1.y, 0);
                            (*triangles[n])[1].color = color2;
                            color2.a = alphaPI2;
                            (*triangles[n])[2].position = Vector3f(pointI2.x,pointI2.y, 0);
                            (*triangles[n])[1].color = color2;
                            addTriangle(quad);
                        } else {
                            int alphaP1 = (*triangles[n])[1].color.a;
                            int alphaP2 = (*triangles[n])[2].color.a;
                            int alphaPI1 = compAlphaFromPoint(pointI1);
                            int alphaPI2 = compAlphaFromPoint(pointI2);
                            VertexArray *quad = new VertexArray(Quads);
                            quad->append(Vertex((*triangles[n])[0].position, color));
                            color2.a = alphaP1;
                            quad->append(Vertex((*triangles[n])[1].position, color2));
                            color2.a = alphaP2;
                            quad->append(Vertex((*triangles[n])[2].position, color2));
                            color2.a = alphaPI2;
                            quad->append(Vertex(Vector3f(pointI2.x, pointI2.y, 0), color2));
                            color2.a = alphaPI2;
                            (*triangles[n])[1].position = Vector3f(pointI2.x,pointI2.y, 0);
                            (*triangles[n])[1].color = color2;
                            color2.a = alphaPI1;
                            (*triangles[n])[2].position = Vector3f(pointI1.x,pointI1.y, 0);
                            (*triangles[n])[1].color = color2;
                            addTriangle(quad);
                        }
                    } else {
                        Sinon, si le côté d'intersection est le côté extrême du triangle, cela est un peu plus compliqué.
                        *Il faut vérifier quel point du côté extrême du triangle est le plus proche de l'extrêmité du segment.
                        *Ceci nous permet de savoir dans quel sens on doit découper le triangle.
                        */
                     /*   float dist1 = Vec2f((*triangles[n])[1].position.x,(*triangles[n])[1].position.y).computeDist(pointI2);
                        float dist2 = Vec2f((*triangles[n])[1].position.x,(*triangles[n])[1].position.y).computeDist(pointI1);
                        if (dist1 < dist2) {
                            int alphaP1 = (*triangles[n])[1].color.a;
                            int alphaP2 = (*triangles[n])[2].color.a;
                            int alphaPI1 = compAlphaFromPoint(pointI1);
                            int alphaPI2 = compAlphaFromPoint(pointI2);
                            VertexArray *quad = new VertexArray(Quads);
                            quad->append(Vertex(Vector3f(a.x, a.y, 0), color));
                            color2.a = alphaP1,
                            quad->append(Vertex((*triangles[n])[1].position, color2));
                            color2.a = alphaPI1;
                            quad->append(Vertex(Vector3f(pointI1.x, pointI1.y, 0), color2));
                            color2.a = alphaPI2;
                            quad->append(Vertex(Vector3f(pointI2.x, pointI2.y, 0), color2));
                            (*triangles[n])[1].position = Vector3f(pointI2.x, pointI2.y, 0);
                            (*triangles[n])[1].color = color2;
                            addTriangle(quad);
                        } else {
                            int alphaP1 = (*triangles[n])[1].color.a;
                            int alphaP2 = (*triangles[n])[2].color.a;
                            int alphaPI1 = compAlphaFromPoint(pointI1);
                            int alphaPI2 = compAlphaFromPoint(pointI2);
                            VertexArray *quad = new VertexArray(Quads);
                            quad->append(Vertex(Vector3f(a.x, a.y, 0), color));
                            color2.a = alphaPI2;
                            quad->append(Vertex(Vector3f(pointI2.x, pointI2.y, 0), color2));
                            color2.a = alphaPI1;
                            quad->append(Vertex(Vector3f(pointI1.x, pointI1.y, 0), color2));
                            color2.a = alphaP2;
                            quad->append(Vertex((*triangles[n])[2].position, color2));
                            color2.a = alphaPI2;
                            (*triangles[n])[2].position = Vector3f(pointI2.x, pointI2.y, 0);
                            (*triangles[n])[2].color = color2;
                        }
                    }
                }
            }*/

            void DirectionnalLight::computeCone () {
                for (unsigned int i = 0; i < coneTris.size(); i++) {
                    delete coneTris[i];
                }
                coneTris.clear();
                math::Vec3f center = getLightCenter();
                math::Vec3f centerCone = center - dir * (Light::getHeight() * 0.5f);
                math::Vec3f origCone = center - dir * center;
                TransformMatrix tm;
                tm.setRotation(math::Vec3f(0, 0, 1), 90);
                math::Vec3f ortho = tm.transform(dir);
                math::Vec3f ext1 (center.x - ortho.x * getSize().x * 0.5f, center.y - ortho.y * getSize().y * 0.5f, center.z);
                math::Vec3f ext2 (center.x + ortho.x * getSize().x * 0.5f, center.y + ortho.y * getSize().y * 0.5f, center.z);
                math::Vec3f d1 = ext1 - origCone;
                math::Vec3f d2 = ext2 - origCone;
                d1 = d1.normalize();
                d2 = d2.normalize();
                math::Vec3f extCenter1 = ext1 - d1 * (Light::getHeight() * 0.5f);
                math::Vec3f extCenter2 = ext2 - d2 * (Light::getHeight() * 0.5f);
                VertexArray* s1 = new VertexArray (Triangles);
                Color color1(color.r, color.g, color.b, intensity / 20);
                Color color2 (color.r, color.g, color.b, 0);
                s1->append(Vertex(Vector3f(centerCone.x, centerCone.y, 0), color1));
                s1->append(Vertex(Vector3f(origCone.x, origCone.y, 0), color2));
                s1->append(Vertex(Vector3f(extCenter1.x, extCenter1.y, 0), color2));
                //s1->SetBlendMode(Blend::Add);
                coneTris.push_back(s1);
                VertexArray* s2 = new VertexArray (Triangles);
                s2->append(Vertex(Vector3f(centerCone.x, centerCone.y, 0), color1));
                s2->append(Vertex(Vector3f(origCone.x, origCone.y, 0), color2));
                s2->append(Vertex(Vector3f(extCenter2.x, extCenter2.y, 0), color2));
                //s2->SetBlendMode(Blend::Add);
                coneTris.push_back(s2);
                VertexArray* s3 = new VertexArray (Quads);
                s3->append(Vertex(Vector3f(centerCone.x, centerCone.y, 0), color1));
                s3->append(Vertex(Vector3f(extCenter1.x, extCenter1.y, 0), color2));
                s3->append(Vertex(Vector3f(ext1.x, ext1.y, 0), color2));
                s3->append(Vertex(Vector3f(center.x, center.y, 0), color1));
                //s3->SetBlendMode(Blend::Add);
                coneTris.push_back(s3);
                VertexArray* s4 = new VertexArray (Quads);
                s4->append(Vertex(Vector3f(centerCone.x, centerCone.y, 0), color1));
                s4->append(Vertex(Vector3f(extCenter2.x, extCenter2.y, 0), color2));
                s4->append(Vertex(Vector3f(ext2.x, ext2.y, 0), color2));
                s4->append(Vertex(Vector3f(center.x, center.y, 0), color1));
                //s4->SetBlendMode(Blend::Add);
                coneTris.push_back(s4);
                computeDirLightSize(origCone);
            }
            void DirectionnalLight::computeDirLightSize (math::Vec3f &origCone) {
                math::Vec3f position (getPosition().x, getPosition().y, getPosition().z);
                math::Vec3f center (center.x, center.y, center.z);
                std::array<math::Vec3f, 5> points;
                points[0] = math::Vec3f (position.x - center.x, position.y - center.y, center.z);
                points[1] = math::Vec3f (position.x + center.x, position.y - center.y, center.z);
                points[2] = math::Vec3f (position.x - center.x, position.y + center.y, center.z);
                points[3] = math::Vec3f (position.x + center.x, position.y + center.y, center.z);
                points[4] = origCone;
                std::array<std::array<float, 2>, 3> store;
                store = math::Computer::getExtends(points);
                setSize(math::Vec3f ((int) (store[0][1] - store[0][0]), (int) (store[1][1] - store[1][0]), store[2][1] - store[2][0]));
                position = math::Vec3f (store[0][0], store[1][0], store[2][0]);
            }
            bool DirectionnalLight::operator== (Entity &other) {
                   if (!GameObject::operator==(other))
                       return false;
                   return color == other.getColor() &&
                          intensity == other.getIntensity();
            }
            vector<VertexArray*> DirectionnalLight::getConeTris () {
                return coneTris;
            }
            int DirectionnalLight::compAlphaFromPoint (math::Vec2f &point) {
                math::Vec2f v1 = point -  math::Vec2f(getCenter().x, getCenter().y);
                math::Vec2f v2 = v1.normalize();
                v2.x *= littleRadius * 0.5f;
                v2.y *= bigRadius * 0.5f;
                float m = v1.magnSquared() / v2.magnSquared();
                return 255 - (int) (255.f * m);
            }

            DirectionnalLight::~DirectionnalLight () {
                for (unsigned int i = 0; i < triangles.size(); i++) {
                    delete triangles[i];
                }
                triangles.clear();
                for (unsigned int i = 0; i < coneTris.size(); i++) {
                    delete coneTris[i];
                }
                coneTris.clear();

            }
        }
    }
}


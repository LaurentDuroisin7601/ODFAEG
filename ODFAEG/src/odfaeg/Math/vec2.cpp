#include "../../../include/odfaeg/Math/vec2f.h"
#include "../../../include/odfaeg/Math/vec4.h"
namespace odfaeg {
    namespace math {
        using namespace std;
        using namespace sf;
        const Vec2f Vec2f::xAxis = Vec2f (1, 0);
        const Vec2f Vec2f::yAxis = Vec2f (0, 1);
        ostream& operator<< (ostream &out, const Vec2f &vec2) {
            out<<"x : "<<vec2.x<<" y : "<<vec2.y<<endl;
            return out;
        }
        Vec2f::Vec2f () : Vector2f (0.f, 0.f) {

        }

        Vec2f::Vec2f(float x, float y) : Vector2f (x, y) {

        }
        Vec2f::Vec2f(const Vec3f& other) {
            x = other.x;
            y = other.y;
        }
        bool Vec2f::isNulVector () const {
            return x==0 && y==0;
        }
        Vec2f Vec2f::operator- () const {
            return Vec2f (-this->x, -this->y);
        }
        Vec2f Vec2f::operator+ (const Vec2f &other) {

            Vector2f vec2 = this->getVec2sf() + other.getVec2sf();
            return Vec2f (vec2.x, vec2.y);

        }
        Vec2f Vec2f::operator- (const Vec2f &other) {
            sf::Vector2f vec2 = this->getVec2sf() - other.getVec2sf();
            return Vec2f (vec2.x, vec2.y);
        }
        Vec2f Vec2f::operator* (const Vec2f &other) {
            float x = this->x * other.x;
            float y = this->y * other.y;
            return Vec2f (x, y);
        }
        Vec2f Vec2f::operator* (const float scale) {
            sf::Vector2f vec2 = this->getVec2sf() * scale;
            return Vec2f (vec2.x, vec2.y);
        }
        Vec2f Vec2f::operator/ (const Vec2f &other) {
            try {
                if (other.x == 0 || other.y == 0)
                    throw core::Erreur(0, "Division by zero.", 1);
                else
                    return Vec2f (this->x / other.x, this->y / other.y);
            } catch (core::Erreur err) {
                cerr<<err.what()<<endl;
            }
            return Vec2f(0, 0);
        }
        Vec2f& Vec2f::operator+= (const Vec2f &other) {
            this->x += other.x;
            this->y += other.y;
            return *this;
        }
        Vec2f& Vec2f::operator-= (const Vec2f &other) {

           this->x -= other.x;
           this->y -= other.y;
           return *this;
        }
        Vec2f& Vec2f::operator*= (const Vec2f &other) {

           this->x *= other.x;
           this->y *= other.y;
           return *this;
        }
        Vec2f& Vec2f::operator*= (const float scale) {

           this->x *= scale;
           this->y *= scale;
           return *this;
        }
        Vec2f Vec2f::operator/= (const Vec2f &other) {
            try {
                if (other.x == 0 || other.y == 0)
                    throw core::Erreur(0, "Division by zero.", 1);
                else {
                    this->x /= other.x;
                    this->y /= other.y;
                    return *this;
                }
            } catch (core::Erreur err) {
                cerr<<err.what()<<endl;
            }
            return Vec2f();
        }


        Vec2f& Vec2f::operator= (const Vec2f &other) {
            x = other.x;
            y = other.y;
            return *this;
        }
        Vec2f& Vec2f::operator= (const Vec3f &other) {
            x = other.x;
            y = other.y;
            return *this;
        }
        bool Vec2f::operator== (const Vec2f &other) {
            return x == other.x && y == other.y;
        }

        float Vec2f::computeDist (const Vec2f &other) {
            return Math::sqrt(Math::power(x - other.x, 2) + Math::power(y - other.y, 2));
        }
        float Vec2f::magnitude () const {
            return Math::sqrt(Math::power(x, 2) + Math::power(y, 2));
        }
        float Vec2f::magnSquared () {
            return Math::power(x, 2) + Math::power(y, 2);
        }
        Vec2f Vec2f::normalize () const {
            float length = magnitude();
            Vec2f v2f;
            if (length==0) {
                v2f = Vec2f(0.f, 0.f);
            } else {
                float x = this->x / length;
                float y = this->y / length;
                v2f = Vec2f(x, y);
            }

            return v2f;
        }
        Vec2f Vec2f::operator/ (float scalar) {
            return Vec2f (this->x / scalar, this->y / scalar);
        }
        float Vec2f::dot2 (const Vec2f &other) {
            if (isNulVector ())
                return 0.f;
            Vec2f v (x + other.x, y + other.y);
            return 0.5f * Math::power(v.magnitude(), 2) - Math::power(magnitude(), 2);
        }
        float Vec2f::dot (const Vec2f &other) const {
            if(isNulVector ())
                return 0;
            Vec2f n1 = *this;
            Vec2f n2 = other;
            if (n1.magnitude() > 1)
                n1 = n1.normalize ();
            if (n2.magnitude () > 1)
                n2 = n2.normalize ();

            return n1.x * n2.x + n1.y * n2.y;
        }
        float Vec2f::getAngleBetween (const Vec2f &other) {
            if (isNulVector() || other.isNulVector() || *this == other)
                return 0;
            if (*this == -other)
                return PI;
            float dotProduct = dot(other);
            Vec3f v1 (x, y, 0);
            Vec3f v2 (other.x, other.y, 0);
            Vec3f v3 = v1.cross(v2);
            if (v3.z >= 0)
                return Math::acosinus(dotProduct);
            else
                return -Math::acosinus(dotProduct);
        }

        float Vec2f::projOnAxis (const Vec2f &other) {

            float dp = this->dot(other);
            return dp * other.magnitude();
        }
        Vector2f Vec2f::getVec2sf () const {
            return Vector2f(x, y);
        }
        float Vec2f::operator[] (int i) {
            if (i == 0)
                return x;
            else
                return y;
        }
        bool Vec2f::operator!= (const Vec2f& v) {
            return !(*this == v);
        }
        float* Vec2f::Vec2f::getVec2 () const {
            float* vec2 = new float[2];
            vec2[0] = x;
            vec2[1] = y;
            return vec2;
        }
        bool Vec2f::isOpposite (const Vec2f &other) const {
            if (isNulVector() || other.isNulVector())
                return false;
            if (((x > 0.f && other.x < 0.f) || (x < 0.f && other.x > 0.f))
                && ((y > 0.f && other.y < 0.f) || (y < 0.f && other.y > 0.f)))
                return true;
            return false;
        }
    }
}

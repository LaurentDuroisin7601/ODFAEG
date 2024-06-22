#include "../../../include/odfaeg/Math/vec4.h"
#include "../../../include/odfaeg/Math/matrix4.h"
namespace odfaeg {
    namespace math {
        using namespace std;
        using namespace sf;

        const Vec3f Vec3f::xAxis(1.f, 0.f, 0.f);
        const Vec3f Vec3f::yAxis(0.f, 1.f, 0.f);
        const Vec3f Vec3f::zAxis(0.f, 0.f, 1.f);

        ostream& operator<< (ostream &out, const Vec3f &vec3) {
            out<<"x : "<<vec3.x<<" y : "<<vec3.y<<" z : "<<vec3.z<<" w : "<<vec3.w<<endl;
            return out;
        }
        Vec3f::Vec3f () : Vector3f (0.f, 0.f, 0.f) {
            w = 1.f;
        }

        Vec3f::Vec3f(float x, float y, float z) : Vector3f (x, y, z) {
            w = 1.f;
        }
        Vec3f::Vec3f (float x, float y, float z, float w) : Vector3f(x, y, z) {
            this->w = w;
        }
        Vec3f::Vec3f(const Vec2f &other) {
            x = other.x;
            y = other.y;
            z = 0;
            w = 1;
        }
        void Vec3f::set (float x, float y, float z) {
            this->x = x;
            this->y = y;
            this->z = z;
        }

        Vec3f Vec3f::normalizeToVec3 () const {

            Vector3f vec3 = getVec3sf();
            if (w != 0.f) {
                return Vec3f (vec3.x / w, vec3.y / w, vec3.z / w);

            }
            else
                return *this;
        }
        bool Vec3f::isNulVector () const {
            return x==0 && y==0 && z==0;
        }
        Vec3f Vec3f::operator- () const {
            return Vec3f (-this->x, -this->y, -this->z);
        }
        Vec3f Vec3f::operator+ (const Vec3f &other) {

            Vector3f vec3 = this->getVec3sf() + other.getVec3sf();
            return Vec3f (vec3.x, vec3.y, vec3.z, w);

        }
        Vec3f Vec3f::operator- (const Vec3f &other) {
            sf::Vector3f v1 = this->getVec3sf();
            sf::Vector3f v2 = other.getVec3sf();
            sf::Vector3f vec3 = v1 - v2;
            return Vec3f (vec3.x, vec3.y, vec3.z);
        }
        Vec3f Vec3f::operator* (const Vec3f &other) {
            float x = this->x * other.x;
            float y = this->y * other.y;
            float z = this->z * other.z;
            return Vec3f (x, y, z, w);
        }
        Vec3f Vec3f::operator* (const float scale) {
            sf::Vector3f v1 = this->getVec3sf();
            sf::Vector3f vec3 = v1 * scale;
            return Vec3f (vec3.x, vec3.y, vec3.z, w * scale);
        }
        Vec3f Vec3f::operator/ (const float scale) {
            sf::Vector3f v1 = this->getVec3sf();
            sf::Vector3f vec3 = v1 / scale;
            return Vec3f (vec3.x, vec3.y, vec3.z, w / scale);
        }
        Vec3f Vec3f::operator/ (const Vec3f &other) {
            try {
                if (other.x == 0 || other.y == 0 || other.z == 0)
                    throw core::Erreur(0, "Division by zero.", 1);
                else
                    return Vec3f (this->x / other.x, this->y / other.y, this->z / other.z);
            } catch (core::Erreur err) {
                cerr<<err.what()<<endl;
            }
            return Vec3f();
        }
        Vec3f Vec3f::operator+ (const float number) {
            Vec3f vec;
            vec.x = this->x + number;
            vec.y = this->y + number;
            vec.z = this->z + number;
            return vec;

        }
        Vec3f& Vec3f::operator+= (const Vec3f &other) {

            this->x += other.x;
            this->y += other.y;
            this->z += other.z;
            return *this;

        }
        Vec3f& Vec3f::operator-= (const Vec3f &other) {
           this->x -= other.x;
           this->y -= other.y;
           this->z -= other.z;
           return *this;
        }
        Vec3f& Vec3f::operator*= (const Vec3f &other) {
           this->x *= other.x;
           this->y *= other.y;
           this->z *= other.z;
           return *this;
        }
        Vec3f& Vec3f::operator*= (const float scale) {
           this->x *= scale;
           this->y *= scale;
           this->z *= scale;
           return *this;
        }
        Vec3f& Vec3f::operator/= (const Vec3f &other) {
            try {
                if (other.x == 0 || other.y == 0 || other.z == 0)
                    throw core::Erreur(0, "Division by zero.", 1);
                else {
                    this->x /= other.x;
                    this->y /= other.y;
                    this->z /= other.z;
                }
            } catch (core::Erreur err) {
                cerr<<err.what()<<endl;
            }
            return *this;
        }
        Vec3f& Vec3f::operator= (const Vec3f &other) {
            x = other.x;
            y = other.y;
            z = other.z;
            w = other.w;
            return *this;
        }
        Vec3f& Vec3f::operator= (const Vec2f& other) {
            x = other.x;
            y = other.y;
            z = 0;
            w = 1;
            return *this;
        }
        bool Vec3f::operator== (const Vec3f &other) {
            return x==other.x && y==other.y && z==other.z;
        }
        bool Vec3f::operator != (const Vec3f& other) {
            return !(*this == other);
        }
        float Vec3f::computeDist (const Vec3f &other) {
            return Math::sqrt(Math::power(x - other.x, 2) + Math::power(y - other.y, 2) + Math::power(z - other.z, 2));
        }
        float Vec3f::computeDistSquared(const Vec3f &other) {
            return Math::power(x - other.x, 2) + Math::power(y - other.y, 2) + Math::power(z - other.z, 2);
        }
        float Vec3f::magnitude () const {
            return Math::sqrt(Math::power(x, 2) + Math::power(y, 2) + Math::power(z, 2));
        }
        float Vec3f::magnSquared () const {
            return Math::power(x, 2) + Math::power(y, 2) + Math::power(z, 2);
        }
        Vec3f Vec3f::normalize () const {
            float length = magnitude();

            Vec3f v3f;
            if (length==0) {
                v3f.set(0.f, 0.f, 0.f);
            } else {
                float x = this->x / length;
                float y = this->y / length;
                float z = this->z / length;
                v3f.set (x, y, z);
            }

            return v3f;
        }
        float Vec3f::dot2 (const Vec3f &other) {
            if (isNulVector ())
                return 0.f;
            return x * other.x + y * other.y + z * other.z;

        }
        float Vec3f::dot3(const Vec3f &other) {
            Vec3f v (x + other.x, y + other.y, z + other.z);
            return 0.5f * Math::power(v.magnitude(), 2) - Math::power(magnitude(), 2) - Math::power(other.magnitude(), 2);
        }
        float Vec3f::dot (const Vec3f &other) const {
            if(isNulVector ())
                return 0.f;
           Vec3f n1 = normalize();
           Vec3f n2 = other.normalize();
           return (n1.x * n2.x + n1.y * n2.y + n1.z * n2.z);
        }
        float Vec3f::getAngleBetween (const Vec3f &other, const Vec3f &n) {
            if(isNulVector() || other.isNulVector())
                return 0;
            float cosinus = dot(other);
            if (cosinus == 1)
                return 0;
            if (cosinus == -1)
                return PI;
            Vec3f crs = cross(other);
            if (n.dot(crs) < 0)
                return Math::acosinus(cosinus);
            return -Math::acosinus(cosinus);
        }
        Vec3f Vec3f::cross (const Vec3f &other) const {

            return Vec3f (y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
        }
        float Vec3f::projOnAxis (const Vec3f &other) {
            return dot(other) * magnitude();
        }
        Vec3f Vec3f::projOnVector(Vec3f other) {
            Vec3f proj;
            float dp = dot2(other);
            proj.x = dp / other.magnSquared() * other.x;
            proj.y = dp / other.magnSquared() * other.y;
            proj.z = dp / other.magnSquared() * other.z;
            return proj;
        }
        bool Vec3f::isOpposite (const Vec3f &other) const {
            if (isNulVector() || other.isNulVector())
                return false;
            if (((x > 0.f && other.x < 0.f) || (x < 0.f && other.x > 0.f))
                && ((y > 0.f && other.y < 0.f) || (y < 0.f && other.y > 0.f))
                && ((z > 0.f && other.z < 0.f) || (z < 0.f && other.z > 0.f)))
                return true;
            return false;
        }
        float* Vec3f::getVec3 () const {
            float* v = new float[3];
            v[0] = x;
            v[1] = y;
            v[2] = z;
            return v;
        }
        Vector3f Vec3f::getVec3sf () const {
            return Vector3f(x, y, z);
        }

        Vec3f Vec3f::fabs () const {
            float x = Math::abs(this->x);
            float y = Math::abs(this->y);
            float z = Math::abs(this->z);
            return Vec3f (x, y, z);
        }
        float& Vec3f::operator[](unsigned int i) {
            if (i == 0)
                return x;
            else if (i == 1)
                return y;
            else if (i == 2)
                return z;
            else
                return w;
        }
    }
}







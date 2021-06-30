#include "../../../include/odfaeg/Math/matrix3.h"
#include "../../../include/odfaeg/Math/matrix4.h"
//Matrice 3*3 de flottants.
namespace odfaeg {
    namespace math {
        using namespace std;
        Matrix3f::Matrix3f  () {
            m11 = m22 = m33 = 1;
            m12 = m13 = m21 = m23 = m31 = m32 = 0;
        }
        Matrix3f::Matrix3f  (Matrix4f matrix4f) {
            m11 = matrix4f.m11;
            m12 = matrix4f.m12;
            m13 = matrix4f.m13;
            m21 = matrix4f.m21;
            m22 = matrix4f.m22;
            m23 = matrix4f.m23;
            m31 = matrix4f.m31;
            m32 = matrix4f.m32;
            m33 = matrix4f.m33;
        }
        //Créer une matrice avec les valeurs.
        Matrix3f::Matrix3f (float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33) {
            this->m11 = m11;
            this->m12 = m12;
            this->m13 = m13;
            this->m21 = m21;
            this->m22 = m22;
            this->m23 = m23;
            this->m31 = m31;
            this->m32 = m32;
            this->m33 = m33;
        }

        void Matrix3f::set(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33) {
            this->m11 = m11;
            this->m12 = m12;
            this->m13 = m13;
            this->m21 = m21;
            this->m22 = m22;
            this->m23 = m23;
            this->m31 = m31;
            this->m32 = m32;
            this->m33 = m33;
        }
        //Charge la matrice nullptre.
        void Matrix3f::zero () {
            m11 = m12 = m13 = m21 = m22 = m23 = m31 = m32 = m33 = 0;
        }
        //Charge la matrice identité.
        void Matrix3f::identity () {
            zero ();
            m11 = m22 = m33 = 1.f;
        }
        //Additionne deux matrices.
        Matrix3f Matrix3f::operator+ (const Matrix3f &other) {
            float m11 = this->m11 + other.m11;
            float m12 = this->m12 + other.m12;
            float m13 = this->m13 + other.m13;

            float m21 = this->m21 + other.m21;
            float m22 = this->m22 + other.m22;
            float m23 = this->m23 + other.m23;

            float m31 = this->m31 + other.m31;
            float m32 = this->m32 + other.m32;
            float m33 = this->m33 + other.m33;
            return Matrix3f (m11, m12, m13, m21, m22, m23, m31, m32, m33);

        }
        //Soustrait deux matrices.
        Matrix3f Matrix3f::operator- (const Matrix3f &other) {
            float m11 = this->m11 - other.m11;
            float m12 = this->m12 - other.m12;
            float m13 = this->m13 - other.m13;

            float m21 = this->m21 - other.m21;
            float m22 = this->m22 - other.m22;
            float m23 = this->m23 - other.m23;

            float m31 = this->m31 - other.m31;
            float m32 = this->m32 - other.m32;
            float m33 = this->m33 - other.m33;
            return Matrix3f (m11, m12, m13, m21, m22, m23, m31, m32, m33);

        }
        //Multiplie deux matrices.
        Matrix3f Matrix3f::operator*(const Matrix3f &other) {
            float m11 = this->m11 * other.m11 + this->m12 * other.m21 + this->m13 * other.m31;
            float m12 = this->m11 * other.m12 + this->m12 * other.m22 + this->m13 * other.m32;
            float m13 = this->m11 * other.m13 + this->m12 * other.m23 + this->m13 * other.m33;


            float m21 = this->m21 * other.m11 + this->m22 * other.m21 + this->m23 * other.m31;
            float m22 = this->m21 * other.m12 + this->m22 * other.m22 + this->m23 * other.m32;
            float m23 = this->m21 * other.m13 + this->m22 * other.m23 + this->m23 * other.m33;


            float m31 = this->m31 * other.m11 + this->m32 * other.m21 + this->m33 * other.m31;
            float m32 = this->m31 * other.m12 + this->m32 * other.m22 + this->m33 * other.m32;
            float m33 = this->m31 * other.m13 + this->m32 * other.m23 + this->m33 * other.m33;



            return Matrix3f (m11, m12, m13, m21, m22, m23, m31, m32, m33);

        }
        //Multiplie un vecteur à une matrice.
        Vec3f Matrix3f::operator* (const Vec3f &vec) {
            float x = m11 * vec.x + m12 * vec.y + m13 * vec.z;
            float y = m21 * vec.x + m22 * vec.y + m23 * vec.z;
            float z = m31 * vec.x + m32 * vec.y + m33 * vec.z;
            return Vec3f (x, y, z);

        }
        Vec2f Matrix3f::operator* (const Vec2f &vec) {
            float x = m11 * vec.x + m12 * vec.y + m13;
            float y = m21 * vec.x + m22 * vec.y + m23;
            return Vec2f (x, y);
        }
        //Multiplie un vecteur à une matrice, renvoie un vecteur à 2 dimensions.
        Vec2f Matrix3f::mult (const Vec3f &vec3) {
            float x = m11 * vec3.x + m12 * vec3.y + m13 * vec3.z;
            float y = m21 * vec3.x + m22 * vec3.y + m23 * vec3.z;

            return Vec2f (x, y);
        }

        //Multiplie une matrice par un scalaire.
        Matrix3f Matrix3f::operator* (const float scalar) {
            float m11 = this->m11 * scalar;
            float m12 = this->m12 * scalar;
            float m13 = this->m13 * scalar;

            float m21 = this->m21 * scalar;
            float m22 = this->m22 * scalar;
            float m23 = this->m23 * scalar;

            float m31 = this->m31 * scalar;
            float m32 = this->m32 * scalar;
            float m33 = this->m33 * scalar;

            Matrix3f m3f (m11, m12, m13, m21, m22, m23, m31, m32, m33);
            return m3f;
        }
        Matrix3f& Matrix3f::operator= (const Matrix3f &other) {
            m11 = other.m11;
            m12 = other.m12;
            m13 = other.m13;
            m21 = other.m21;
            m22 = other.m22;
            m23 = other.m23;
            m31 = other.m31;
            m32 = other.m32;
            m33 = other.m33;
            return *this;
        }
        //Calcule le déterminant d'une matrice.
        float Matrix3f::getDet () const {
            float fCo11 = m22 * m33 - m23 * m32;
            float fCo21 = m23 * m31 - m21 * m33;
            float fCo31 = m21 * m32 - m22 * m31;
            float fDet = m11 * fCo11 + m12 * fCo21 + m13 * fCo31;
            return fDet;
        }
        //Inverse une matrice.
        Matrix3f Matrix3f::inverse () {
            Matrix3f store (m11, m12, m13, m21, m22, m23, m31, m32, m33);
            float det = getDet();
            if (Math::abs(det) < 0.f)
                return store;
            else {
                store.m11 = m22*m33 - m23*m32;
                store.m12 = m13*m32 - m12*m33;
                store.m13 = m12*m23 - m13*m22;
                store.m21 = m23*m31 - m21*m33;
                store.m22 = m11*m33 - m13*m31;
                store.m23 = m13*m21 - m11*m23;
                store.m31 = m21*m32 - m22*m31;
                store.m32 = m12*m31 - m11*m32;
                store.m33 = m11*m22 - m12*m21;
            }
            float invDet = 1.f / det;
            return store * invDet;
        }
        ostream& operator<< (ostream &out, const Matrix3f &mat3) {
            out<<mat3.m11<<" "<<mat3.m12<<" "<<mat3.m13<<endl;
            out<<mat3.m21<<" "<<mat3.m22<<" "<<mat3.m23<<endl;
            out<<mat3.m31<<" "<<mat3.m32<<" "<<mat3.m33<<endl;
            return out;
        }
    }
}

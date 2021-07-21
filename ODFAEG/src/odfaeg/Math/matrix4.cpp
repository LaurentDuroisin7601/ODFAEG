#include "../../../include/odfaeg/Math/matrix4.h"
#include <array>
/*Classe créant une matrice 4*4.*/
namespace odfaeg {
    namespace math {
        using namespace std;
        Matrix4f::Matrix4f () {
            m11 = m22 = m33 = m44 = 1;
            m12 = m13 = m14 = m21 = m23 = m24 = m41 = m42 = m43 = 0;
        }
        Matrix4f::Matrix4f (Matrix3f matrix3f) {
            m11 = matrix3f.m11;
            m12 = matrix3f.m12;
            m13 = matrix3f.m13;
            m14 = 0;
            m21 = matrix3f.m21;
            m22 = matrix3f.m22;
            m23 = matrix3f.m23;
            m24 = 0;
            m31 = matrix3f.m31;
            m32 = matrix3f.m32;
            m33 = matrix3f.m33;
            m34 = 0;
            m41 = 0;
            m42 = 0;
            m43 = 0;
            m44 = 1;
        }
        //Créer une matrice 4*4.
        Matrix4f::Matrix4f (float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24,
            float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44) {
                this->m11 = m11;
                this->m12 = m12;
                this->m13 = m13;
                this->m14 = m14;
                this->m21 = m21;
                this->m22 = m22;
                this->m23 = m23;
                this->m24 = m24;
                this->m31 = m31;
                this->m32 = m32;
                this->m33 = m33;
                this->m34 = m34;
                this->m41 = m41;
                this->m42 = m42;
                this->m43 = m43;
                this->m44 = m44;
        }
        void Matrix4f::setM4f (float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24,
            float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44) {
            this->m11 = m11;
            this->m12 = m12;
            this->m13 = m13;
            this->m14 = m14;
            this->m21 = m21;
            this->m22 = m22;
            this->m23 = m23;
            this->m24 = m24;
            this->m31 = m31;
            this->m32 = m32;
            this->m33 = m33;
            this->m34 = m34;
            this->m41 = m41;
            this->m42 = m42;
            this->m43 = m43;
            this->m44 = m44;
        }
        //Charge la matrice nullptre.
        void Matrix4f::zero () {
            m11 = m12 = m13 = m14 = m21 = m22 = m23 = m24 = m31 = m32 = m33 = m34 = m41 = m42 = m43 = m44 = 0.f;
        }
        //Charge la matrice neutre.
        void Matrix4f::identity () {
            zero ();
            m11 = m22 = m33 = m44 = 1.f;
        }
        //Pareil que dans matrix 3f mais avec une matrice 4*4 cette fois.
        Matrix4f Matrix4f::operator+ (const Matrix4f &other) {
            float m11 = this->m11 + other.m11;
            float m12 = this->m12 + other.m12;
            float m13 = this->m13 + other.m13;
            float m14 = this->m14 + other.m14;
            float m21 = this->m21 + other.m21;
            float m22 = this->m22 + other.m22;
            float m23 = this->m23 + other.m23;
            float m24 = this->m24 + other.m24;
            float m31 = this->m31 + other.m31;
            float m32 = this->m32 + other.m32;
            float m33 = this->m33 + other.m33;
            float m34 = this->m34 + other.m34;
            float m41 = this->m41 + other.m41;
            float m42 = this->m42 + other.m42;
            float m43 = this->m43 + other.m43;
            float m44 = this->m44 + other.m44;
            Matrix4f m4f(m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44);
            return m4f;

        }
        Matrix4f Matrix4f::operator-(const Matrix4f &other) {
            float m11 = this->m11 - other.m11;
            float m12 = this->m12 - other.m12;
            float m13 = this->m13 - other.m13;
            float m14 = this->m14 - other.m14;
            float m21 = this->m21 - other.m21;
            float m22 = this->m22 - other.m22;
            float m23 = this->m23 - other.m23;
            float m24 = this->m24 - other.m24;
            float m31 = this->m31 - other.m31;
            float m32 = this->m32 - other.m32;
            float m33 = this->m33 - other.m33;
            float m34 = this->m34 - other.m34;
            float m41 = this->m41 - other.m41;
            float m42 = this->m42 - other.m42;
            float m43 = this->m43 - other.m43;
            float m44 = this->m44 - other.m44;
            Matrix4f m4f (m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44);
            return m4f;
        }
        Matrix4f Matrix4f::operator*(const Matrix4f &other) {
            float m11 = this->m11 * other.m11 + this->m12 * other.m21 + this->m13 * other.m31 + this->m14 * other.m41;
            float m12 = this->m11 * other.m12 + this->m12 * other.m22 + this->m13 * other.m32 + this->m14 * other.m42;
            float m13 = this->m11 * other.m13 + this->m12 * other.m23 + this->m13 * other.m33 + this->m14 * other.m43;
            float m14 = this->m11 * other.m14 + this->m12 * other.m24 + this->m13 * other.m34 + this->m14 * other.m44;

            float m21 = this->m21 * other.m11 + this->m22 * other.m21 + this->m23 * other.m31 + this->m24 * other.m41;
            float m22 = this->m21 * other.m12 + this->m22 * other.m22 + this->m23 * other.m32 + this->m24 * other.m42;
            float m23 = this->m21 * other.m13 + this->m22 * other.m23 + this->m23 * other.m33 + this->m24 * other.m43;
            float m24 = this->m21 * other.m14 + this->m22 * other.m24 + this->m23 * other.m34 + this->m24 * other.m44;

            float m31 = this->m31 * other.m11 + this->m32 * other.m21 + this->m33 * other.m31 + this->m34 * other.m41;
            float m32 = this->m31 * other.m12 + this->m32 * other.m22 + this->m33 * other.m32 + this->m34 * other.m42;
            float m33 = this->m31 * other.m13 + this->m32 * other.m23 + this->m33 * other.m33 + this->m34 * other.m43;
            float m34 = this->m31 * other.m14 + this->m32 * other.m24 + this->m33 * other.m34 + this->m34 * other.m44;

            float m41 = this->m41 * other.m11 + this->m42 * other.m21 + this->m43 * other.m31 + this->m44 * other.m41;
            float m42 = this->m41 * other.m12 + this->m42 * other.m22 + this->m43 * other.m32 + this->m44 * other.m42;
            float m43 = this->m41 * other.m13 + this->m42 * other.m23 + this->m43 * other.m33 + this->m44 * other.m43;
            float m44 = this->m41 * other.m14 + this->m42 * other.m24 + this->m43 * other.m34 + this->m44 * other.m44;
            Matrix4f m4f (m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44);
            return m4f;
        }
        Vec3f Matrix4f::operator* (const Vec3f &vec) {
            float x = m11 * vec.x + m12 * vec.y + m13 * vec.z + m14 * vec.w;
            float y = m21 * vec.x + m22 * vec.y + m23 * vec.z + m24 * vec.w;
            float z = m31 * vec.x + m32 * vec.y + m33 * vec.z + m34 * vec.w;
            float w = m41 * vec.x + m42 * vec.y + m43 * vec.z + m44 * vec.w;

            return Vec3f(x, y, z, w);
        }
        Matrix4f Matrix4f::operator* (float scalar) {
            float m11 = this->m11 * scalar;
            float m12 = this->m12 * scalar;
            float m13 = this->m13 * scalar;
            float m14 = this->m14 * scalar;

            float m21 = this->m21 * scalar;
            float m22 = this->m22 * scalar;
            float m23 = this->m23 * scalar;
            float m24 = this->m24 * scalar;

            float m31 = this->m31 * scalar;
            float m32 = this->m32 * scalar;
            float m33 = this->m33 * scalar;
            float m34 = this->m34 * scalar;

            float m41 = this->m41 * scalar;
            float m42 = this->m42 * scalar;
            float m43 = this->m43 * scalar;
            float m44 = this->m44 * scalar;
            Matrix4f m4f (m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44);
            return m4f;
        }
        //Calcule le déterminant. (Utilisation de la méthode des co-facteurs et du mineur.)
        float Matrix4f::getDet() {
            float det12 = m31 * m42 - m32 * m41;
            float det13 = m31 * m43 - m33 * m41;
            float det14 = m31 * m44 - m34 * m41;
            float det23 = m32 * m43 - m33 * m42;
            float det24 = m32 * m44 - m34 * m42;
            float det34 = m33 * m44 - m34 * m43;
            return m11 * (m11 * det34 - m23 * det24 + m24 * det23) - m12
            * (m21 * det34 - m23 * det14 + m24 * det13) + m13
            * (m21 * det24 - m22 * det14 + m24 * det12) - m14
            * (m21 * det23 - m22 * det13 + m23 * det12);
        }
        Matrix4f Matrix4f::inverse () {
            Matrix4f store;
            float fa1 = m11 * m22 - m12 * m21;
            float fa2 = m11 * m23 - m13 * m21;
            float fa3 = m11 * m24 - m14 * m21;
            float fa4 = m12 * m23 - m13 * m22;
            float fa5 = m12 * m24 - m14 * m22;
            float fa6 = m13 * m24 - m14 * m23;
            float fb1 = m31 * m42 - m32 * m41;
            float fb2 = m31 * m43 - m33 * m41;
            float fb3 = m31 * m44 - m34 * m41;
            float fb4 = m32 * m43 - m33 * m42;
            float fb5 = m32 * m44 - m34 * m42;
            float fb6 = m33 * m44 - m34 * m43;
            float det = fa1 * fb6 - fa2 * fb5 + fa3 * fb4 + fa4 * fb3 - fa5 * fb2 + fa6 * fb1;
            if (Math::abs(det) <= 0.f) {
                return store;
            } else {
               store.m11 = m22 * fb6 - m23 * fb5 + m24 * fb4;
               store.m21 = -m21 * fb6 + m23 * fb3 - m24 * fb2;
               store.m31 = m21 * fb5 - m22 * fb3 + m24 * fb1;
               store.m41 = -m21 * fb4 + m22 * fb2 - m23 * fb1;

               store.m12 = -m12 * fb6 + m13 * fb5 - m14 * fb4;
               store.m22 = m11 * fb6 - m13 * fb3 + m14 * fb2;
               store.m32 = -m11 * fb5 + m12 * fb3 - m14 * fb1;
               store.m42 = m11 * fb4 - m12 * fb2 + m13 * fb1;

               store.m13 = m42 * fa6 - m43 * fa5 + m44 * fa4;
               store.m23 = -m41 * fa6 + m43 * fa3 - m44 * fa2;
               store.m33 = m41 * fa5 - m42 * fa3 + m44 * fa1;
               store.m43 = -m41 * fa4 + m42 * fa2 - m43 * fa1;

               store.m14 = -m32 * fa6 + m33 * fa5 - m34 * fa4;
               store.m24 = m31 * fa6 - m33 * fa3 + m34 * fa2;
               store.m34 = -m31 * fa5 + m32 * fa3 - m34 * fa1;
               store.m44 = m31 * fa4 - m32 * fa2 + m33 * fa1;


               float invDet = 1.f / det;
               return store * invDet;
            }
        }
        Matrix4f& Matrix4f::operator= (const Matrix4f &other) {
            m11 = other.m11; m12 = other.m12; m13 = other.m13; m14 = other.m14;
            m21 = other.m21; m22 = other.m22; m23 = other.m23; m24 = other.m24;
            m31 = other.m31; m32 = other.m32; m33 = other.m33; m34 = other.m34;
            m41 = other.m41; m42 = other.m42; m43 = other.m43; m44 = other.m44;
            return *this;
        }
        bool Matrix4f::operator== (const Matrix4f &other) {
            return m11 == other.m11 &&
                   m12 == other.m12 &&
                   m13 == other.m13 &&
                   m14 == other.m14 &&
                   m21 == other.m21 &&
                   m22 == other.m22 &&
                   m23 == other.m23 &&
                   m24 == other.m24 &&
                   m31 == other.m31 &&
                   m32 == other.m32 &&
                   m33 == other.m33 &&
                   m34 == other.m34 &&
                   m41 == other.m41 &&
                   m42 == other.m42 &&
                   m43 == other.m43 &&
                   m44 == other.m44;
        }
        Matrix4f Matrix4f::transpose() {
            Matrix4f matrix;
            matrix.m11 = m11;
            matrix.m12 = m21;
            matrix.m13 = m31;
            matrix.m14 = m41;
            matrix.m21 = m12;
            matrix.m22 = m22;
            matrix.m23 = m32;
            matrix.m24 = m42;
            matrix.m31 = m13;
            matrix.m32 = m23;
            matrix.m33 = m33;
            matrix.m34 = m43;
            matrix.m41 = m14;
            matrix.m42 = m24;
            matrix.m43 = m34;
            matrix.m44 = m44;
            return matrix;
        }
        std::array<float, 16> Matrix4f::toGlMatrix() {
            std::array<float, 16> matrix;
            matrix[0] = m11;
            matrix[1] = m12;
            matrix[2] = m13;
            matrix[3] = m14;
            matrix[4] = m21;
            matrix[5] = m22;
            matrix[6] = m23;
            matrix[7] = m24;
            matrix[8] = m31;
            matrix[9] = m32;
            matrix[10] = m33;
            matrix[11] = m34;
            matrix[12] = m41;
            matrix[13] = m42;
            matrix[14] = m43;
            matrix[15] = m44;
            return matrix;
        }
        ostream& operator<< (ostream &out, const Matrix4f &mat4) {
            out<<mat4.m11<<" "<<mat4.m12<<" "<<mat4.m13<<" "<<mat4.m14<<std::endl;
            out<<mat4.m21<<" "<<mat4.m22<<" "<<mat4.m23<<" "<<mat4.m24<<std::endl;
            out<<mat4.m31<<" "<<mat4.m32<<" "<<mat4.m33<<" "<<mat4.m34<<std::endl;
            out<<mat4.m41<<" "<<mat4.m42<<" "<<mat4.m43<<" "<<mat4.m44<<std::endl;
            return out;
        }
        istream& operator>> (istream &in, Matrix4f &mat4) {
            in>>mat4.m11;
            in>>mat4.m12;
            in>>mat4.m13;
            in>>mat4.m14;
            in>>mat4.m21;
            in>>mat4.m22;
            in>>mat4.m23;
            in>>mat4.m24;
            in>>mat4.m31;
            in>>mat4.m32;
            in>>mat4.m33;
            in>>mat4.m34;
            in>>mat4.m41;
            in>>mat4.m42;
            in>>mat4.m43;
            in>>mat4.m44;
            return in;
        }
    }
}



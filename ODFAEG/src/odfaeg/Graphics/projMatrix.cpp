#include "../../../include/odfaeg/Graphics/projMatrix.h"
/*Matrice de projection*/
namespace odfaeg {
    namespace graphic {
        ProjMatrix::ProjMatrix () {

        }
        //Réinitialise la matrice de projection.
        void ProjMatrix::reset () {
            matrix4f.identity ();
        }

        void ProjMatrix::setGlOrthoMatrix (double l, double r, double b, double t, double n, double f) {
            matrix4f.m11 = 2 / (r - l);
            matrix4f.m12 = 0;
            matrix4f.m13 = 0;
            matrix4f.m14 = -((r + l) / (r - l));
            matrix4f.m21 = 0;
            matrix4f.m22 = -2 / (t - b);
            matrix4f.m23 = 0;
            matrix4f.m24 = -((t + b) / (t - b));
            matrix4f.m31 = 0;
            matrix4f.m32 = 0;
            matrix4f.m33 = -2 / (f - n);
            matrix4f.m34 = -((f + n) / (f - n));
            matrix4f.m41 = 0;
            matrix4f.m42 = 0;
            matrix4f.m43 = 0;
            matrix4f.m44 = 1;
            this->l = l;
            this->r = r;
            this->b = b;
            this->t = t;
            this->n = n;
            this->f = f;
            invMatrix4f = matrix4f.inverse();
        }
        void ProjMatrix::setGlPerspectiveMatrix (double l, double r, double b, double t, double n, double f) {
            matrix4f.m11 = (2 * n) / (r - l);
            matrix4f.m12 = 0;
            matrix4f.m13 = (r + l) / (r - l);
            matrix4f.m14 = 0;
            matrix4f.m21 = 0;
            matrix4f.m22 = (2 * n) / (t - b);
            matrix4f.m23 = (t + b) / (t - b);
            matrix4f.m24 = 0;
            matrix4f.m31 = 0;
            matrix4f.m32 = 0;
            matrix4f.m33 = -((f + n) / (f - n));
            matrix4f.m34 = (-2 * f * n) / (f - n);
            matrix4f.m41 = 0;
            matrix4f.m42 = 0;
            matrix4f.m43 = -1;
            matrix4f.m44 = 0;
            this->l = l;
            this->r = r;
            this->b = b;
            this->t = t;
            this->n = n;
            this->f = f;
            invMatrix4f.inverse();
        }
        float* ProjMatrix::getGlMatrix () {
            float* matrix = new float[16];
            matrix[0] = matrix4f.m11;
            matrix[1] = matrix4f.m21;
            matrix[2] = matrix4f.m31;
            matrix[3] = matrix4f.m41;
            matrix[4] = matrix4f.m12;
            matrix[5] = matrix4f.m22;
            matrix[6] = matrix4f.m32;
            matrix[7] = matrix4f.m42;
            matrix[8] = matrix4f.m13;
            matrix[9] = matrix4f.m23;
            matrix[10] = matrix4f.m33;
            matrix[11] = matrix4f.m43;
            matrix[12] = matrix4f.m14;
            matrix[13] = matrix4f.m24;
            matrix[14] = matrix4f.m34;
            matrix[15] = matrix4f.m44;
            return matrix;
        }
        math::Matrix4f ProjMatrix::getMatrix() {
            return matrix4f;
        }
        //Projette un vecteur suivant la matrice de projection définie.
        math::Vec3f ProjMatrix::project (math::Vec3f vec3) {
            math::Matrix4f mat = matrix4f;
            mat.m22 = -mat.m22;
            return matrix4f * vec3;
        }
        //Déprojette un vecteur suivant la matrice de projection définie.
        math::Vec3f ProjMatrix::unProject (math::Vec3f vec3f) {
            math::Matrix4f mat = matrix4f;
            mat.m22 = -mat.m22;
            return matrix4f.inverse() * vec3f;
        }
        physic::BoundingBox ProjMatrix::getFrustum() {
            return physic::BoundingBox(l,b,0,(r - l),(t - b), f);
        }
    }
}

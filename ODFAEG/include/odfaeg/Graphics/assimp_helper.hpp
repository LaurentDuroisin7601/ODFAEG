#ifndef ODFAEG_ASSIMP_HELPER_HPP
#define ODFAEG_ASSIMP_HELPER_HPP
#include<assimp/vector3.h>
#include<assimp/matrix4x4.h>
#include<glm/glm.hpp>
#include<glm/gtc/quaternion.hpp>
#include "../Math/quaternion.hpp"
namespace odfaeg {
    namespace graphic {
        class AssimpHelpers
        {
        public:

            static inline glm::mat4 convertMatrixToGLMFormat(const aiMatrix4x4& from)
            {
                glm::mat4 to;
                //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
                to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
                to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
                to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
                to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
                return to;
            }

            static inline glm::vec3 getGLMVec(const aiVector3D& vec)
            {
                return glm::vec3(vec.x, vec.y, vec.z);
            }

            static inline glm::quat getGLMQuat(const aiQuaternion& pOrientation)
            {
                return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
            }
            static inline math::Matrix4f convertAssimpToODFAEGNodeTransformMatrix(aiMatrix4x4 aiMatrix) {
                math::Matrix4f mat;
                mat[0][0] = aiMatrix.a1;
                mat[0][1] = aiMatrix.a2;
                mat[0][2] = aiMatrix.a3;
                mat[0][3] = aiMatrix.a4;

                mat[1][0] = aiMatrix.b1;
                mat[1][1] = aiMatrix.b2;
                mat[1][2] = aiMatrix.b3;
                mat[1][3] = aiMatrix.b4;

                mat[2][0] = aiMatrix.c1;
                mat[2][1] = aiMatrix.c2;
                mat[2][2] = aiMatrix.c3;
                mat[2][3] = aiMatrix.c4;

                mat[3][0] = aiMatrix.d1;
                mat[3][1] = aiMatrix.d2;
                mat[3][2] = aiMatrix.d3;
                mat[3][3] = aiMatrix.d4;
                return mat;
            }
            static inline math::Matrix4f convertAssimpToODFAEGOffsetMatrix (aiMatrix4x4 aiMatrix) {
                math::Matrix4f mat;
                mat[0][0] = aiMatrix.a1;
                mat[0][1] = aiMatrix.b1;
                mat[0][2] = aiMatrix.c1;
                mat[0][3] = aiMatrix.d1;

                mat[1][0] = aiMatrix.a2;
                mat[1][1] = aiMatrix.b2;
                mat[1][2] = aiMatrix.c2;
                mat[1][3] = aiMatrix.d2;

                mat[2][0] = aiMatrix.a3;
                mat[2][1] = aiMatrix.b3;
                mat[2][2] = aiMatrix.c3;
                mat[2][3] = aiMatrix.d3;

                mat[3][0] = aiMatrix.a4;
                mat[3][1] = aiMatrix.b4;
                mat[3][2] = aiMatrix.c4;
                mat[3][3] = aiMatrix.d4;
                return mat;
            }
            static inline math::Vec3f convertAssimpToODFAEGVec4(aiVector3D aiVec) {
                math::Vec3f vec;
                vec[0] = aiVec.x;
                vec[1] = aiVec.y;
                vec[2] = aiVec.z;
                return vec;
            }
            static inline math::Quaternion convertAssimpToODFAEGQuaternion(aiQuaternion aiQuat) {
                math::Quaternion quat;
                quat.x = aiQuat.x;
                quat.y = aiQuat.y;
                quat.z = aiQuat.z;
                quat.w = aiQuat.w;
                return quat;
            }
        };
    }
}
#endif // ODFAEG_ASSIMP_HELPER_HPP

#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <glm/glm.hpp>
#include <GL/glew.h>


class Material {

    public:

        Material(glm::vec3 color, float Ka, float Kd, float Ks, float alpha);

        static void setUniformLocations(GLint uni_K, GLint uni_alpha, GLint uni_color);

        void sendUniforms();

    private:
        glm::vec3 color;
        float Ka, Kd, Ks, alpha;
        static GLint uni_K, uni_alpha, uni_color;
};


#endif // MATERIAL_H_

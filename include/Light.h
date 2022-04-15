#ifndef LIGHT_H_
#define LIGHT_H_

#include <glm/glm.hpp>
#include <GL/glew.h>

class Light {

    public:
        Light(glm::vec3 dir, glm::vec3 col) : direction(dir), color(col), camSpaceDirection(dir) {}

        void sendUniforms();
        void setView(glm::mat4 const& v);

        static void setUniformLocations(GLint uni_dir, GLint uni_color);

    private:
        glm::vec3 direction;
        glm::vec3 camSpaceDirection;
        glm::vec3 color;
        static GLint uni_dir, uni_color;
};

#endif // LIGHT_H_

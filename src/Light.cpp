#include "Light.h"

GLint Light::uni_color, Light::uni_dir;

void Light::sendUniforms()
{
    glUniform3f(uni_color, color.r, color.g, color.b);
    glUniform3f(uni_dir, camSpaceDirection.x, camSpaceDirection.y, camSpaceDirection.z);
}

void Light::setUniformLocations(GLint uni_dir, GLint uni_color)
{
    Light::uni_color = uni_color;
    Light::uni_dir = uni_dir;
}

void Light::setView(glm::mat4 const& v)
{
    camSpaceDirection = glm::normalize(glm::vec3(v * glm::vec4(direction, 0.f)));
}

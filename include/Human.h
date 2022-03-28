#ifndef HUMAN_H_
#define HUMAN_H_

#include <stack>
#include <vector>
#include <GL/glew.h>

#include "Cone.h"
#include "Cylinder.h"
#include "Cube.h"
#include "Sphere.h"
#include "Shader.h"

class BodyPart
{
    public:
        BodyPart(Geometry* g, glm::mat4 m);
        ~BodyPart();

        glm::mat4 getMatrix() const { return matrix; }
        const std::vector<BodyPart*>& getChilds() { return childs; }
        void setMatrix(glm::mat4 const& m) { matrix = m; }
        BodyPart* addChild(BodyPart* bp) { childs.push_back(bp); return bp; }

        void Render(GLint uMV);

        static std::stack<glm::mat4> matrices;
    private:
        GLuint vaoID = 0;
        GLuint vboID = 0;
        unsigned int nbVert;
        std::vector<BodyPart*> childs;
        glm::mat4 matrix;
};

class Human
{
    public:
        Human();
        ~Human();

        void Render(float t);

    private:
        BodyPart* root;
        Shader* shader;
        glm::mat4 projection;
        glm::mat4 view;
        GLint uMV;
};


#endif // HUMAN_H_

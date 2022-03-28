#include "Human.h"
#include "logger.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <GL/glew.h>

std::stack<glm::mat4> BodyPart::matrices;

const float PI = 3.141592;

BodyPart::BodyPart(Geometry* g, glm::mat4 m)
{
    matrix = m;

    // if this body part has geometry, create a vertex buffer and fill it
    if (g != nullptr)
    {
        nbVert = g->getNbVertices();
        glGenVertexArrays(1, &vaoID);
        glBindVertexArray(vaoID);

        glGenBuffers(1, &vboID);
        glBindBuffer(GL_ARRAY_BUFFER, vboID);

            glBufferData(GL_ARRAY_BUFFER, (3 + 3) * sizeof(float)*nbVert, nullptr, GL_DYNAMIC_DRAW);

            glBufferSubData(GL_ARRAY_BUFFER, 0,                       sizeof(float)*3*nbVert, g->getVertices());
            glBufferSubData(GL_ARRAY_BUFFER, 3*sizeof(float)*nbVert,  3*sizeof(float)*nbVert, g->getNormals());

            glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
            glVertexAttribPointer(1, 3, GL_FLOAT, 0, 0, (void*)(nbVert*3*sizeof(float)));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

BodyPart::~BodyPart()
{
    // cleanup
    if (vaoID != 0) glDeleteVertexArrays(1, &vaoID);
    if (vboID != 0) glDeleteBuffers(1, &vboID);
    for (auto c : childs) delete c;
    childs.clear();
}

void BodyPart::Render(GLint uMV)
{
    // push matrix
    if (matrices.empty()) matrices.push(matrix);
    else matrices.push(matrices.top() * matrix);

    // render self
    if (vaoID != 0)
    {
        glBindVertexArray(vaoID);
        glUniformMatrix4fv(uMV, 1, false, glm::value_ptr(matrices.top()));
        glDrawArrays(GL_TRIANGLES, 0, nbVert);
        glBindVertexArray(0);
    }

    // render all childs
    for (auto c : childs) c->Render(uMV);

    // pop matrix
    matrices.pop();
}


Human::Human() : root(new BodyPart(nullptr, glm::mat4(1.f)))
{
    // create the shader and get the uniform for the model-view matrix
    FILE* vertexFile = fopen("Shaders/color.vert", "r");
    FILE* fragmentFile = fopen("Shaders/color.frag", "r");
    shader = Shader::loadFromFiles(vertexFile, fragmentFile);
    fclose(vertexFile);
    fclose(fragmentFile);
    if (shader == nullptr)
    {
        ERROR("failed to load shaders");
        exit(1);
    }
    uMV = glGetUniformLocation(shader->getProgramID(), "uMV");

    // set the projection matrix once
    projection = glm::perspective(PI/4.f, 800.f/600.f, 0.01f, 10000.f);
    GLint uP = glGetUniformLocation(shader->getProgramID(), "uP");

    glUseProgram(shader->getProgramID());
    glUniformMatrix4fv(uP, 1, false, glm::value_ptr(projection));
    glUseProgram(0);

    // create the camera matrix
    view = glm::lookAt(glm::vec3(4.f, 4.f, 4.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

    // the body is a cube scaled along the y axis
    glm::mat4 matBody = glm::mat4(1.f);
    matBody = glm::scale(matBody, glm::vec3(1.f, 1.5f, 1.f));
    parts.insert({"body", root->addChild(new BodyPart(new Cube(), matBody))});

    // the head is a small sphere positionned at the top of the body
    glm::mat4 matHead = glm::mat4(1.f);
    matHead = glm::translate(matHead, glm::vec3(0.f, 1.f, 0.f)); // translate it last
    matHead = glm::scale(matHead, glm::vec3(0.5f)); // scale it first
    parts.insert({"head", root->addChild(new BodyPart(new Sphere(32,32), matHead))});

    // the arms are cylinders scaled and rotated along the top
    glm::mat4 matArm1 = glm::mat4(1.f);
    matArm1 = glm::translate(matArm1, glm::vec3(0.625f, 0.75f, 0.f)); // translate it where it should be
    matArm1 = glm::rotate(matArm1, PI/2, glm::vec3(1.f, 0.f, 0.f)); // rotate it accordingly
    matArm1 = glm::translate(matArm1, glm::vec3(0.f, 0.f, 0.5f)); // move the center of rotation
    matArm1 = glm::scale(matArm1, glm::vec3(0.25f, 0.25f, 1.f)); // scale it
    parts.insert({"arm1", root->addChild(new BodyPart(new Cylinder(32), matArm1))});

    // same with second arm
    glm::mat4 matArm2 = glm::mat4(1.f);
    matArm2 = glm::translate(matArm1, glm::vec3(-0.625f, 0.75f, 0.f)); // translate it where it should be (opposed to the other arm)
    matArm2 = glm::rotate(matArm1, PI/2, glm::vec3(1.f, 0.f, 0.f)); // rotate it accordingly
    matArm2 = glm::translate(matArm1, glm::vec3(0.f, 0.f, 0.5f)); // move the center of rotation
    matArm2 = glm::scale(matArm1, glm::vec3(0.25f, 0.25f, 1.f)); // scale it
    parts.insert({"arm2", root->addChild(new BodyPart(new Cylinder(32), matArm2))});

    // the forearms are cones
    glm::mat4 matForearms = glm::mat4(1.f); // same matrix for now
    matForearms = glm::translate(matForearms, glm::vec3(0.f, 0.f, 1.f)); // TODO: animate
    parts.insert({"forearm1", parts["arm1"]->addChild(new BodyPart(new Cone(32, .25), matForearms))});
    parts.insert({"forearm2", parts["arm2"]->addChild(new BodyPart(new Cone(32, .25), matForearms))});

    // TODO: legs
}

Human::~Human()
{
    delete shader;
    delete root;
}

void Human::Render(float t)
{
    // animate (TODO)
    float sint = glm::sin(t*4);
    root->setMatrix(glm::rotate(glm::mat4(1.f), t/2.f, glm::vec3(0,1,0)));
    parts["arm1"]->setMatrix(glm::scale(glm::translate(glm::rotate(glm::translate(glm::mat4(1.f), glm::vec3(0.625, 0.75, 0)),11*PI/24.f + sint*PI/8.f, glm::vec3(1,0,0)),glm::vec3(0,0,0.5)),glm::vec3(.25,.25,1)));
    parts["arm2"]->setMatrix(glm::scale(glm::translate(glm::rotate(glm::translate(glm::mat4(1.f), glm::vec3(-0.625, 0.75, 0)),11*PI/24.f - sint*PI/8.f, glm::vec3(1,0,0)),glm::vec3(0,0,0.5)),glm::vec3(.25,.25,1)));

    // push the view matrix then render everything
    BodyPart::matrices.push(view);
    glUseProgram(shader->getProgramID());
    root->Render(uMV);
    glUseProgram(0);
    BodyPart::matrices.pop();
}

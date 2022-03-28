#include "Human.h"
#include "logger.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <GL/glew.h>

std::stack<glm::mat4> BodyPart::matrices;

BodyPart::BodyPart(Geometry* g, glm::mat4 m)
{
    matrix = m;
    if (g != nullptr)
    {
        nbVert = g->getNbVertices();
        glGenVertexArrays(1, &vaoID);
        glBindVertexArray(vaoID);

        glGenBuffers(1, &vboID);
        glBindBuffer(GL_ARRAY_BUFFER, vboID);

            glBufferData(GL_ARRAY_BUFFER, (3 + 3) * sizeof(float)*nbVert, NULL, GL_DYNAMIC_DRAW);

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
    if (vaoID != 0) glDeleteVertexArrays(1, &vaoID);
    if (vboID != 0) glDeleteBuffers(1, &vboID);
    for (auto c : childs) delete c;
    childs.clear();
}

void BodyPart::Render(GLint uMV)
{
    //push matrix
    if (matrices.empty()) matrices.push(matrix);
    else matrices.push(matrices.top() * matrix);
    //render self
    if (vaoID != 0)
    {
        glBindVertexArray(vaoID);
        glUniformMatrix4fv(uMV, 1, false, glm::value_ptr(matrices.top()));
        glDrawArrays(GL_TRIANGLES, 0, nbVert);
        glBindVertexArray(0);
    }
    //render childs
    for (auto c : childs) c->Render(uMV);
    //pop matrix
    matrices.pop();
}


Human::Human() : root(new BodyPart(nullptr, glm::mat4(1.f)))
{
    FILE* vertexFile = fopen("Shaders/color.vert", "r");
    FILE* fragmentFile = fopen("Shaders/color.frag", "r");
    shader = Shader::loadFromFiles(vertexFile, fragmentFile);
    fclose(vertexFile);
    fclose(fragmentFile);
    if (shader == NULL)
    {
        ERROR("failed to load shaders");
        exit(1);
    }
    //body = BodyPart((Geometry*)new Cube(), glm::mat4(1.f));
    uMV = glGetUniformLocation(shader->getProgramID(), "uMV");
    GLint uP = glGetUniformLocation(shader->getProgramID(), "uP");
    glUseProgram(shader->getProgramID());
    projection = glm::perspective(3.141592f/4.f, 800.f/600.f, 0.01f, 10000.f);
    glUniformMatrix4fv(uP, 1, false, glm::value_ptr(projection));
    glUseProgram(0);
    view = glm::lookAt(glm::vec3(4, 4, 4), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    root->addChild(new BodyPart(new Cube(), glm::scale(glm::mat4(1.f), glm::vec3(1, 1.5, 1))));
    root->addChild(new BodyPart(new Sphere(32,32), glm::scale(glm::translate(glm::mat4(1.f), glm::vec3(0,1,0)),glm::vec3(0.5))));
    auto arm1 = root->addChild(new BodyPart(new Cylinder(32),
                                glm::scale(
                                        glm::translate(
                                            glm::rotate(
                                            glm::translate(glm::mat4(1.f), glm::vec3(0.625, 0.75, 0)),
                                            3.141592f/2.f, glm::vec3(1,0,0)),
                                        glm::vec3(0,0,0.5)),
                                glm::vec3(.25,.25,1))));
    arm1->addChild(new BodyPart(new Cone(32, .25),
                                glm::translate(
                                    glm::rotate(glm::translate(glm::mat4(1.f), glm::vec3(0,0,0.5)),
                                    0.f, glm::vec3(1,0,0)),
                                    glm::vec3(0,0,0.5))
                                ));
    auto arm2 = root->addChild(new BodyPart(new Cylinder(32),
                                glm::scale(
                                        glm::translate(
                                            glm::rotate(
                                            glm::translate(glm::mat4(1.f), glm::vec3(-0.625, 0.75, 0)),
                                            3.141592f/2.f, glm::vec3(1,0,0)),
                                        glm::vec3(0,0,0.5)),
                                glm::vec3(.25,.25,1))));
    arm2->addChild(new BodyPart(new Cone(32, .25),
                                glm::translate(
                                    glm::rotate(glm::translate(glm::mat4(1.f), glm::vec3(0,0,0.5)),
                                    0.f, glm::vec3(1,0,0)),
                                    glm::vec3(0,0,0.5))
                                ));
}

Human::~Human()
{
    delete shader;
    delete root;
}

void Human::Render(float t)
{
    //UPDATE MATRICES
    float sint = glm::sin(t*2);
    float PI = 3.141592;
    root->setMatrix(glm::rotate(glm::mat4(1.f), t/2.f, glm::vec3(0,1,0)));
    root->getChilds()[2]->setMatrix(
        glm::scale(
                glm::translate(
                    glm::rotate(
                    glm::translate(glm::mat4(1.f), glm::vec3(0.625, 0.75, 0)),
                    11*PI/24.f + sint*PI/8.f, glm::vec3(1,0,0)),
                glm::vec3(0,0,0.5)),
        glm::vec3(.25,.25,1))
    );
    root->getChilds()[3]->setMatrix(
        glm::scale(
                glm::translate(
                    glm::rotate(
                    glm::translate(glm::mat4(1.f), glm::vec3(-0.625, 0.75, 0)),
                    11*PI/24.f - sint*PI/8.f, glm::vec3(1,0,0)),
                glm::vec3(0,0,0.5)),
        glm::vec3(.25,.25,1))
    );
    BodyPart::matrices.push(view);
    glUseProgram(shader->getProgramID());
    root->Render(uMV);
    glUseProgram(0);
    BodyPart::matrices.pop();
}

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

            glBufferData(GL_ARRAY_BUFFER, (3 + 3 + 2) * sizeof(float)*nbVert, nullptr, GL_DYNAMIC_DRAW);

            glBufferSubData(GL_ARRAY_BUFFER, 0,                       sizeof(float)*3*nbVert, g->getVertices());
            glBufferSubData(GL_ARRAY_BUFFER, 3*sizeof(float)*nbVert,  3*sizeof(float)*nbVert, g->getNormals());
            glBufferSubData(GL_ARRAY_BUFFER, 2*3*sizeof(float)*nbVert,  2*sizeof(float)*nbVert, g->getUVs());

            glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
            glVertexAttribPointer(1, 3, GL_FLOAT, 0, 0, (void*)(nbVert*3*sizeof(float)));
            glVertexAttribPointer(2, 2, GL_FLOAT, 0, 0, (void*)(nbVert*3*2*sizeof(float)));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);

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
    delete tex;
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
        if (tex != nullptr) tex->bind();
        glUniformMatrix4fv(uMV, 1, false, glm::value_ptr(matrices.top()));
        glDrawArrays(GL_TRIANGLES, 0, nbVert);
        if (tex != nullptr) tex->unbind();
        glBindVertexArray(0);
    }

    // render all childs
    for (auto c : childs) c->Render(uMV);

    // pop matrix
    matrices.pop();
}


Human::Human() : root(new BodyPart(nullptr, glm::mat4(1.f)))
{
    material = new Material({0,0,1}, 0.2, 0.8, 0.3, 5);
    light = new Light({0, 0.1f, 1}, {1, 1, 1});

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
    GLint uCamPos = glGetUniformLocation(shader->getProgramID(), "uCamPos");
    GLint uP = glGetUniformLocation(shader->getProgramID(), "uP");
    GLint uMatCol = glGetUniformLocation(shader->getProgramID(), "uMatCol");
    GLint uMatK = glGetUniformLocation(shader->getProgramID(), "uMatK");
    GLint uMatAlpha = glGetUniformLocation(shader->getProgramID(), "uMatAlpha");
    GLint uLigCol = glGetUniformLocation(shader->getProgramID(), "uLigCol");
    GLint uLigDir = glGetUniformLocation(shader->getProgramID(), "uLigDir");
    Material::setUniformLocations(uMatK, uMatAlpha, uMatCol);
    Light::setUniformLocations(uLigDir, uLigCol);

    // set the projection matrix once
    projection = glm::perspective(PI/4.f, 800.f/600.f, 0.01f, 10000.f);

    // create the camera matrix
    view = glm::lookAt(glm::vec3(4.f, 4.f, 4.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

    // set the constant uniforms
    glUseProgram(shader->getProgramID());
    glUniformMatrix4fv(uP, 1, false, glm::value_ptr(projection));
    glUniform3f(uCamPos, 4, 4, 4);
    material->sendUniforms();
    glUseProgram(0);

    // display a sphere
    glm::mat4 matSphere = glm::mat4(1.f);
    parts.insert({"sphere", root->addChild(new BodyPart(new Sphere(32, 32), matSphere))});
    parts["sphere"]->setTexture(new Texture("Assets/earth.jpg"));

    glm::mat4 matCube = glm::translate(glm::mat4(1.f), glm::vec3(2,0,0));
    parts.insert({"cube", root->addChild(new BodyPart(new Sphere(32, 32), matCube))});
    parts["cube"]->setTexture(new Texture("Assets/moon.jpg"));

}

Human::~Human()
{
    delete shader;
    delete root;
}

void Human::Render(float t)
{
    parts["cube"]->setMatrix(glm::scale(glm::translate(glm::rotate(glm::mat4(1.f), t/2.f, glm::vec3(.2,1,0)), glm::vec3(0,0,2)),glm::vec3(0.5f)));
    parts["sphere"]->setMatrix(glm::rotate(glm::rotate(glm::mat4(1.f), 0.2f, glm::vec3(0,0,1)), t/1.2f, glm::vec3(0,1,0)));

    // push the view matrix then render everything
    BodyPart::matrices.push(view);
    glUseProgram(shader->getProgramID());
    light->setView(view);
    light->sendUniforms();
    root->Render(uMV);
    glUseProgram(0);
    BodyPart::matrices.pop();
}

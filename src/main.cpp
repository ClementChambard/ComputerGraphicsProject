//SDL Libraries
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

//OpenGL Libraries
#include <GL/glew.h>
#include <GL/gl.h>

//GML libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#include "logger.h"

#define WIDTH     800
#define HEIGHT    600
#define FRAMERATE 60
#define TIME_PER_FRAME_MS  (1.0f/FRAMERATE * 1e3)
#define INDICE_TO_PTR(x) ((void*)(x))

int main(int argc, char *argv[])
{
    ////////////////////////////////////////
    //SDL2 / OpenGL Context initialization : 
    ////////////////////////////////////////
    
    //Initialize SDL2
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        ERROR("The initialization of the SDL failed : %s\n", SDL_GetError());
        return 0;
    }

    //Create a Window
    SDL_Window* window = SDL_CreateWindow("VR Camera",                           //Titre
                                          SDL_WINDOWPOS_UNDEFINED,               //X Position
                                          SDL_WINDOWPOS_UNDEFINED,               //Y Position
                                          WIDTH, HEIGHT,                         //Resolution
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN); //Flags (OpenGL + Show)

    //Initialize OpenGL Version (version 3.0)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    //Initialize the OpenGL Context (where OpenGL resources (Graphics card resources) lives)
    SDL_GLContext context = SDL_GL_CreateContext(window);

    //Tells GLEW to initialize the OpenGL function with this version
    glewExperimental = GL_TRUE;
    glewInit();


    //Start using OpenGL to draw something on screen
    glViewport(0, 0, WIDTH, HEIGHT); //Draw on ALL the screen

    //The OpenGL background color (RGBA, each component between 0.0f and 1.0f)
    glClearColor(0.0, 0.0, 0.0, 1.0); //Full Black

    glEnable(GL_DEPTH_TEST); //Active the depth test

    //TODO
    //From here you can load your OpenGL objects, like VBO, Shaders, etc.
    //TODO

    unsigned int bufferSize = 18;
    float buffer[] = { -1, -1, 0,
                        1, -1, 0,
                        0,  1, 0,
                        1,  0, 0,
                        0,  1, 0,
                        0,  0, 1 };
    unsigned int nbVertices = 3;

    bool isOpened = true;

    FILE* vertShader = fopen("Shaders/color.vert", "r");
    FILE* fragShader = fopen("Shaders/color.frag", "r");

    Shader* shader = Shader::loadFromFiles(vertShader, fragShader);

    fclose(vertShader);
    fclose(fragShader);

    if (shader == NULL)
    {
        ERROR("Shader failed to load\n");
        return EXIT_FAILURE;
    }

    GLuint vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

      GLuint bufferID;
      glGenBuffers(1, &bufferID);
      glBindBuffer(GL_ARRAY_BUFFER, bufferID);

        glBufferData(GL_ARRAY_BUFFER, bufferSize*sizeof(float), buffer, GL_STATIC_DRAW);
        //GLint vPosition = glGetAttribLocation(shader->getProgramID(), "vPosition");
        //GLint vColor = glGetAttribLocation(shader->getProgramID(), "vColor");
        glVertexAttribPointer(/*vPosition*/ 0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribPointer(/*vColor*/ 1, 3, GL_FLOAT, GL_FALSE, 0, INDICE_TO_PTR(3*nbVertices*sizeof(float)));
        glEnableVertexAttribArray(/*vPosition*/ 0);
        glEnableVertexAttribArray(/*vColor*/ 1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    GLint uMVP = glGetUniformLocation(shader->getProgramID(), "uMVP");

    glm::mat4 matrix = glm::mat4(1.f);
    matrix = glm::translate(matrix, glm::vec3(0.5f, 0.f, 0.f));
    matrix = glm::scale(matrix, glm::vec3(0.5f));

    //Main application loop
    while(isOpened)
    {
        //Time in ms telling us when this frame started. Useful for keeping a fix framerate
        uint32_t timeBegin = SDL_GetTicks();

        //Fetch the SDL events
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_WINDOWEVENT:
                    switch(event.window.event)
                    {
                        case SDL_WINDOWEVENT_CLOSE:
                            isOpened = false;
                            break;
                        default:
                            break;
                    }
                    break;
                //We can add more event, like listening for the keyboard or the mouse. See SDL_Event documentation for more details
            }
        }

        //Clear the screen : the depth buffer and the color buffer
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);





        //TODO rendering
        glUseProgram(shader->getProgramID());
        glBindVertexArray(vaoID);
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, &(matrix[0][0]));

        glDrawArrays(GL_TRIANGLES, 0, nbVertices);
        glBindVertexArray(0);
        glUseProgram(0);

        //Display on screen (swap the buffer on screen and the buffer you are drawing on)
        SDL_GL_SwapWindow(window);

        //Time in ms telling us when this frame ended. Useful for keeping a fix framerate
        uint32_t timeEnd = SDL_GetTicks();

        //We want FRAMERATE FPS
        if(timeEnd - timeBegin < TIME_PER_FRAME_MS)
            SDL_Delay((uint32_t)(TIME_PER_FRAME_MS) - (timeEnd - timeBegin));
    }
    
    //Free everything

    delete shader;
    glDeleteBuffers(1, &bufferID);
    glDeleteVertexArrays(1, &vaoID);

    if(context != NULL)
        SDL_GL_DeleteContext(context);
    if(window != NULL)
        SDL_DestroyWindow(window);

    return 0;
}

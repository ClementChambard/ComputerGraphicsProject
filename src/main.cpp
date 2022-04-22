//SDL Libraries
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_syswm.h>

//OpenGL Libraries
#include <GL/glew.h>
#include <GL/gl.h>

//GML libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#include <stack>

#include "Shader.h"

#include "Scene.h"
#include "Camera.h"

#include "logger.h"

#define WIDTH     800
#define HEIGHT    600
#define FRAMERATE 60
#define TIME_PER_FRAME_MS  (1.0f/FRAMERATE * 1e3)
#define INDICE_TO_PTR(x) ((void*)(x))

const float PI = 3.141592;

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

    if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        ERROR("Could not load SDL2_image with PNG files\n");
        return EXIT_FAILURE;
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

    // create the shader and get the uniform for the model-view matrix
    FILE* vertexFile = fopen("Shaders/color.vert", "r");
    FILE* fragmentFile = fopen("Shaders/color.frag", "r");
    Shader* shader = Shader::loadFromFiles(vertexFile, fragmentFile);
    fclose(vertexFile);
    fclose(fragmentFile);
    if (shader == nullptr)
    {
        ERROR("failed to load shaders");
        exit(1);
    }
    GLint uMV = glGetUniformLocation(shader->getProgramID(), "uMV");
    GLint uP = glGetUniformLocation(shader->getProgramID(), "uP");
    GLint uMatCol = glGetUniformLocation(shader->getProgramID(), "uMatCol");
    GLint uMatK = glGetUniformLocation(shader->getProgramID(), "uMatK");
    GLint uMatAlpha = glGetUniformLocation(shader->getProgramID(), "uMatAlpha");
    GLint uLigCol = glGetUniformLocation(shader->getProgramID(), "uLigCol");
    GLint uLigDir = glGetUniformLocation(shader->getProgramID(), "uLigDir");
    Material::setUniformLocations(uMatK, uMatAlpha, uMatCol);
    Light::setUniformLocations(uLigDir, uLigCol);

    // set the projection matrix once
    glm::mat4 projection = glm::perspective(PI/4.f, 800.f/600.f, 0.01f, 10000.f);

    // set the constant uniforms
    glUseProgram(shader->getProgramID());
    glUniformMatrix4fv(uP, 1, false, glm::value_ptr(projection));
    glUseProgram(0);

    // create the light, the camera and the materials
    Light* light = new Light({0, 0.1f, 1}, {1, 1, 1});
    Camera* cam = new Camera();
    Texture* texEarth = new Texture("Assets/earth.jpg");
    Texture* texMoon = new Texture("Assets/moon.jpg");
    Material* materialEarth = new Material(texEarth, 0.2, 0.8, 0, 10);
    Material* materialMoon = new Material(texMoon, 0.2, 0.8, 0.3, 5);

    // create the scene graph and add elements to it
    Scene* sceneGraph = new Scene();

    sceneGraph->setLight(light);

    Sphere sphere(32, 32);

    sceneGraph->addPart("earth", new SceneNode(&sphere, glm::mat4(1.f)));
    sceneGraph->partSetMaterial("earth", materialEarth);

    sceneGraph->addPart("moon", new SceneNode(&sphere, glm::mat4(1.f)));
    sceneGraph->partSetMaterial("moon", materialMoon);

    // the time for animations
    float t = 0;

    bool mouseLock = false;
    bool keyW = false;
    bool keyA = false;
    bool keyS = false;
    bool keyD = false;
    bool keyShift = false;
    bool keySpace = false;
    float mouseX = 0;
    float mouseY = 0;
    bool isOpened = true;
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
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_z: keyW = true; break;
                        case SDLK_q: keyA = true; break;
                        case SDLK_s: keyS = true; break;
                        case SDLK_d: keyD = true; break;
                        case SDLK_SPACE: keySpace = true; break;
                        case SDLK_LSHIFT: keyShift = true; break;
                        case SDLK_LCTRL: SDL_ShowCursor(mouseLock); mouseLock = !mouseLock; break;
                        default: break;
                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_z: keyW = false; break;
                        case SDLK_q: keyA = false; break;
                        case SDLK_s: keyS = false; break;
                        case SDLK_d: keyD = false; break;
                        case SDLK_SPACE: keySpace = false; break;
                        case SDLK_LSHIFT: keyShift = false; break;
                        default: break;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    mouseX = event.motion.x - WIDTH/2.f;
                    mouseY = -event.motion.y + HEIGHT/2.f;
                    break;
                //We can add more event, like listening for the keyboard or the mouse. See SDL_Event documentation for more details
            }
        }

        float pitch = 0.f ,yaw = 0.f;
        if (mouseLock)
        {
            yaw = -mouseX / 100;
            pitch = mouseY / 100;
            SDL_WarpMouseInWindow(window, WIDTH/2.f, HEIGHT/2.f);
        }
        cam->move(pitch, yaw, (keyW - keyS) * .08f, (keyD - keyA) * .08f, (keySpace - keyShift) * .08f);

        //Clear the screen : the depth buffer and the color buffer
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        //TODO rendering
        sceneGraph->setViewMat(cam->getMat());
        sceneGraph->partSetMatrices("moon", glm::scale(glm::translate(glm::rotate(glm::mat4(1.f), t/2.f, glm::vec3(.2,1,0)), glm::vec3(0,0,2)),glm::vec3(0.5f)));
        sceneGraph->partSetMatrices("earth", glm::rotate(glm::rotate(glm::mat4(1.f), 0.2f, glm::vec3(0,0,1)), t/1.2f, glm::vec3(0,1,0)));
        t+=0.01;

        glUseProgram(shader->getProgramID());
        sceneGraph->Render(uMV);
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
    delete sceneGraph;
    delete materialEarth;
    delete materialMoon;
    delete texEarth;
    delete texMoon;
    delete cam;
    delete light;
    if(context != NULL)
        SDL_GL_DeleteContext(context);
    if(window != NULL)
        SDL_DestroyWindow(window);

    return 0;
}

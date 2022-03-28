#version 130
precision mediump float; //Medium precision for float. highp and smallp can also be used

varying vec3 varyNormal; //Sometimes we use "out" instead of "varying". "out" should be used in later version of GLSL.

//We still use varying because OpenGLES 2.0 (OpenGL Embedded System, for example for smartphones) does not accept "in" and "out"

void main()
{
    gl_FragColor = vec4(varyNormal+1.0/2.0, 1.0);
}

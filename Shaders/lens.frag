#version 130
precision mediump float; //Medium precision for float. highp and smallp can also be used

varying vec2 varyUVs;
uniform sampler2D uTex;

void main()
{
    gl_FragColor = texture(uTex, varyUVs);
}

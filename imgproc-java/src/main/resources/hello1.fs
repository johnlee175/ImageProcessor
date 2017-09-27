#version 330 core
precision mediump float;

in vec2 vFragCoord;
out vec4 fragColor;
uniform sampler2D uTexture;

void main() {
    vec4 color = texture(uTexture, vFragCoord);
    fragColor = vec4(1.0 - color.r, 1.0 - color.g, 1.0 - color.b, 1.0);
}
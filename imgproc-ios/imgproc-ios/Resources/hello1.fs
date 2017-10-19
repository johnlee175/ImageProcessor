precision mediump float;

varying vec2 vFragCoord;
uniform sampler2D uTexture;

void main() {
    vec4 color = texture2D(uTexture, vFragCoord);
    gl_FragColor = vec4(1.0 - color.r, 1.0 - color.g, 1.0 - color.b, 1.0);
}

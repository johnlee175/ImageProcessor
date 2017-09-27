#version 330 core
precision mediump float;

in vec2 vFragCoord;
out vec4 fragColor;
uniform sampler2D uTexture;

const vec2 singleStepOffset = vec2(1 / 600.0, 1 / 600.0);
const float params = 1.0 / 8.0;
const vec3 W = vec3(0.299, 0.587, 0.114);

vec2 blurCoordinates[20];

float hardLight(float color) {
	if(color <= 0.5) {
	    color = color * color * 2.0;
	} else {
	    color = 1.0 - ((1.0 - color) * (1.0 - color) * 2.0);
	}
	return color;
}

void main() {
    vec3 centralColor = texture(uTexture, vFragCoord).rgb;
    blurCoordinates[0] = vFragCoord.xy + singleStepOffset * vec2(0.0, -10.0);
    blurCoordinates[1] = vFragCoord.xy + singleStepOffset * vec2(0.0, 10.0);
    blurCoordinates[2] = vFragCoord.xy + singleStepOffset * vec2(-10.0, 0.0);
    blurCoordinates[3] = vFragCoord.xy + singleStepOffset * vec2(10.0, 0.0);
    blurCoordinates[4] = vFragCoord.xy + singleStepOffset * vec2(5.0, -8.0);
    blurCoordinates[5] = vFragCoord.xy + singleStepOffset * vec2(5.0, 8.0);
    blurCoordinates[6] = vFragCoord.xy + singleStepOffset * vec2(-5.0, 8.0);
    blurCoordinates[7] = vFragCoord.xy + singleStepOffset * vec2(-5.0, -8.0);
    blurCoordinates[8] = vFragCoord.xy + singleStepOffset * vec2(8.0, -5.0);
    blurCoordinates[9] = vFragCoord.xy + singleStepOffset * vec2(8.0, 5.0);
    blurCoordinates[10] = vFragCoord.xy + singleStepOffset * vec2(-8.0, 5.0);
    blurCoordinates[11] = vFragCoord.xy + singleStepOffset * vec2(-8.0, -5.0);
    blurCoordinates[12] = vFragCoord.xy + singleStepOffset * vec2(0.0, -6.0);
    blurCoordinates[13] = vFragCoord.xy + singleStepOffset * vec2(0.0, 6.0);
    blurCoordinates[14] = vFragCoord.xy + singleStepOffset * vec2(6.0, 0.0);
    blurCoordinates[15] = vFragCoord.xy + singleStepOffset * vec2(-6.0, 0.0);
    blurCoordinates[16] = vFragCoord.xy + singleStepOffset * vec2(-4.0, -4.0);
    blurCoordinates[17] = vFragCoord.xy + singleStepOffset * vec2(-4.0, 4.0);
    blurCoordinates[18] = vFragCoord.xy + singleStepOffset * vec2(4.0, -4.0);
    blurCoordinates[19] = vFragCoord.xy + singleStepOffset * vec2(4.0, 4.0);

    float sampleColor = centralColor.g * 20.0;
    sampleColor += texture(uTexture, blurCoordinates[0]).g;
    sampleColor += texture(uTexture, blurCoordinates[1]).g;
    sampleColor += texture(uTexture, blurCoordinates[2]).g;
    sampleColor += texture(uTexture, blurCoordinates[3]).g;
    sampleColor += texture(uTexture, blurCoordinates[4]).g;
    sampleColor += texture(uTexture, blurCoordinates[5]).g;
    sampleColor += texture(uTexture, blurCoordinates[6]).g;
    sampleColor += texture(uTexture, blurCoordinates[7]).g;
    sampleColor += texture(uTexture, blurCoordinates[8]).g;
    sampleColor += texture(uTexture, blurCoordinates[9]).g;
    sampleColor += texture(uTexture, blurCoordinates[10]).g;
    sampleColor += texture(uTexture, blurCoordinates[11]).g;
    sampleColor += texture(uTexture, blurCoordinates[12]).g * 2.0;
    sampleColor += texture(uTexture, blurCoordinates[13]).g * 2.0;
    sampleColor += texture(uTexture, blurCoordinates[14]).g * 2.0;
    sampleColor += texture(uTexture, blurCoordinates[15]).g * 2.0;
    sampleColor += texture(uTexture, blurCoordinates[16]).g * 2.0;
    sampleColor += texture(uTexture, blurCoordinates[17]).g * 2.0;
    sampleColor += texture(uTexture, blurCoordinates[18]).g * 2.0;
    sampleColor += texture(uTexture, blurCoordinates[19]).g * 2.0;
    sampleColor = sampleColor / 48.0;

    float highPass = centralColor.g - sampleColor + 0.5;
    for(int i = 0; i < 5; i++) {
        highPass = hardLight(highPass);
    }
    float luminance = dot(centralColor, W);
    float alpha = pow(luminance, params);
    vec3 smoothColor = centralColor + (centralColor - vec3(highPass)) * alpha * 0.1;
    fragColor = vec4(mix(smoothColor.rgb, max(smoothColor, centralColor), alpha), 1.0);
}

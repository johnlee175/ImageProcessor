precision mediump float;

varying vec2 vFragCoord;
uniform sampler2D uTexture;

const vec2 singleStepOffset = vec2(1.0 / 400.0, 1.0 / 400.0);
const float strength = 2.0;

const vec3 W = vec3(0.299, 0.587, 0.114);

void main() {
	float threshold = 0.0;
	//pic1
	vec4 oralColor = texture2D(uTexture, vFragCoord);
	//pic2
	vec3 maxValue = vec3(0.0, 0.0, 0.0);
	for (int i = -2; i <= 2; ++i) {
		for (int j = -2; j <= 2; ++j) {
			vec4 tempColor = texture2D(uTexture, vFragCoord + singleStepOffset * vec2(i, j));
			maxValue.r = max(maxValue.r, tempColor.r);
			maxValue.g = max(maxValue.g, tempColor.g);
			maxValue.b = max(maxValue.b, tempColor.b);
			threshold += dot(tempColor.rgb, W); 
		}
	}
	//pic3
	float gray1 = dot(oralColor.rgb, W);
	//pic4
	float gray2 = dot(maxValue, W);
	//pic5
	float contour = gray1 / gray2;
	threshold = threshold / 25.0;
	float alpha = max(strength, gray1 > threshold ? 1.0 : (gray1 / threshold));
	float result = contour * alpha + (1.0 - alpha) * gray1;
	gl_FragColor = vec4(vec3(result, result, result), oralColor.w);
}

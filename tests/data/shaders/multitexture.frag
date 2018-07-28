varying vec4 color;
varying vec2 texCoord;

uniform sampler2D tex;
uniform sampler2D tex2;

void main(void)
{
	vec4 A = texture2D(tex, texCoord);
	vec4 B = texture2D(tex2, texCoord);
    gl_FragColor = mix(A, color, 1.0 - B.a);
}
varying vec4 color;
varying vec2 texCoord;

uniform sampler2D tex;
uniform float time;

void main(void)
{
	vec4 fcolor = vec4(0.5*(1 + vec3(sin(time), sin(time/5), sin(time/3))), color.a);
    gl_FragColor = texture2D(tex, texCoord) * fcolor;
}
#version 110

uniform vec3 color;

varying vec2 iCornerVect;

void main()
{
	vec4 c;
	c.xyz = (color.xyz / 1.25) + vec3(0.18, 0.18, 0.18);
	c.a = (1.0 - (length(iCornerVect) - 0.25));
	gl_FragColor = c;
}

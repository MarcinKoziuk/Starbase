#version 110

uniform vec3 color;

void main()
{
	gl_FragColor.xyz = (color.xyz / 1.25) + vec3(0.18, 0.18, 0.18);
}

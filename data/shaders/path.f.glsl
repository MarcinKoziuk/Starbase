#version 110

uniform vec3 color;

void main()
{
	gl_FragColor.xyz = (color.xyz / 3.0) + vec3(0.2, 0.2, 0.2);
}

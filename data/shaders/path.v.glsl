#version 110

attribute vec2 position;
attribute vec2 cornerVect;

uniform mat4 mvp;
uniform vec2 scale;
uniform float thickness;
uniform float zoom;

void main()
{
	vec2 dpos = position + (cornerVect/scale * max((thickness * 0.25)/zoom + 0.2, 1.5/zoom));
	vec4 fragCoord = (mvp * vec4(dpos, 1.0, 1.0));

	gl_Position = fragCoord;
}

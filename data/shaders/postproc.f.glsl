#version 110

uniform float time;
uniform sampler2D fboTexture;
varying vec2 f_texcoord;

void main(void)
{
	vec2 texcoord = f_texcoord;
	//texcoord.x += sin(texcoord.y * 4.0*2.0*3.14159 + (time/2.4)) / 100.0;
	gl_FragColor = texture2D(fboTexture, texcoord);
}
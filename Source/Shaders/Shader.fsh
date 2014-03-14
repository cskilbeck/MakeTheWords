//////////////////////////////////////////////////////////////////////
// Inputs

varying lowp vec4 color;
varying lowp vec2 texCoord;

//////////////////////////////////////////////////////////////////////
// Constants/Samplers

uniform sampler2D s_texture;

//////////////////////////////////////////////////////////////////////
// Shader

void main()
{
	gl_FragColor = color * texture2D(s_texture, texCoord);
}

//////////////////////////////////////////////////////////////////////

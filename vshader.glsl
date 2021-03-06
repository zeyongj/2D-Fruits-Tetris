#version 130

in vec4 vPosition;
in vec4 vColor;
out vec4 color;

uniform int xsize;
uniform int ysize;

void main() 
{
	mat4 scale = mat4(2.0/xsize,	0.0,		0.0, 0.0,
					0.0,			2.0/ysize, 	0.0, 0.0,
					0.0, 			0.0, 		1.0, 0.0,
					0.0, 			0.0, 		0.0, 1.0 );

	vec4 newPos = vPosition + vec4(-200, -360, 0, 0);
	gl_Position = scale * newPos; 

	color = vColor;	
} 
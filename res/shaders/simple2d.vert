#version 430 core

// layout range [20,29]

// Grand fathered vetex positions 
in layout(location = 0) vec3 position;
in layout(location = 2) vec2 textureCoordinates_in;

uniform layout(location = 20) mat4 OP;

// Oratagonal projection matrix; 

out layout(location = 30) vec3 position_out;
out layout(location = 31) vec2 textureCoordinates_out;

void main()
{
    position_out = vec3(OP*vec4(position, 1.0f));
    textureCoordinates_out = textureCoordinates_in;
    gl_Position = OP*vec4(position, 1.0f);
}

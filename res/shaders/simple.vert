#version 430 core

// layout range [0,9]

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;

uniform layout(location = 3) mat4 MVP;
uniform layout(location = 4) mat4 MV;
uniform layout(location = 5) mat3 normalMatrix;

out layout(location = 10) vec3 position_out;
out layout(location = 11) vec3 normal_out;
out layout(location = 12) vec2 textureCoordinates_out;

void main()
{
    position_out = vec3(MV * vec4(position, 1.0f));
    normal_out = normalize(normalMatrix * normal_in);
    textureCoordinates_out = textureCoordinates_in;
    gl_Position = MVP * vec4(position, 1.0f);
}

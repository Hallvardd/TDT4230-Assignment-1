#version 430 core

// LOCATION RANGE [30, 39]


in layout(location = 30) vec3 fragmentPosition;
in layout(location = 31) vec2 textureCoordinates;

layout(binding = 0) uniform sampler2D textSampler;

out vec4 textCoord;
out vec4 color;

void main()
{ 
    color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    textCoord = texture(textSampler, textureCoordinates);
}

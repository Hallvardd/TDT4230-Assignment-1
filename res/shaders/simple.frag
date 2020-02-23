#version 430 core

// LOCATION RANGE [10, 19]

struct LightSource{
    vec4 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant; 
    float linear;
    float quadratic;
};

in layout(location = 10) vec3 fragmentPosition; 
in layout(location = 11) vec3 normal;
in layout(location = 12) vec2 textureCoordinates;

out vec4 color;

uniform layout(location = 13) vec4 lightPositions[3]; 

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }


vec3 surface_dc = vec3(0.05f, 0.05f, 0.05f);
vec3 surface_sc = vec3(0.7f, 0.7f, 0.5f);

vec3 N = normalize(normal);
float ambient_i  = 0.0f;
float diffuse_i  = 0.0f;
float specular_i = 0.0f;
vec3  ambient_c  = {0.05f, 0.07f, 0.03f};
vec3  diffuse_c  = {0.05f, 0.0f, 0.0f};
vec3  sepcular_c ;


void main()
{
    // amibient

    // disfuse
    for(int i=0; i<3; i++)
    {
        vec3 L = normalize(lightPositions[i].xyz - fragmentPosition);
        vec3 E = normalize(-fragmentPosition);
        vec3 R = normalize(-reflect(L,N));

        // diffuse color
        diffuse_i = max(dot(N,L), 0.0);
        diffuse_c += diffuse_i * surface_dc;
    }

    // specular

    // attenuation

    //color = vec4(0.5 * N + 0.5, 1.0);

    color = vec4(ambient_c + diffuse_c, 1.0f);
}

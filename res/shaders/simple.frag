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
uniform layout(location = 13) vec3 cameraPos;
uniform layout(location = 14) vec3 ballPos; 
uniform layout(location = 15) vec4 lightPositions[3]; // location 14-16 since 3 positions

uniform LightSource lightSources[3];
out vec4 color;


float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

vec3 reject(vec3 from, vec3 onto)
{
    return from - onto*dot(from, onto)/dot(onto, onto);
}


//
float ballRad = 3.0f; 
vec3 surface_dc = vec3(1.0f, 1.0f, 1.0f);
vec3 surface_sc = vec3(1.0f, 1.0f, 1.0f);

vec3 N = normalize(normal);

float ambient_i  = 0.0f;
float diffuse_i  = 0.0f;
float specular_i = 0.0f;

vec3  ambient_c  = {0.0f, 0.0f, 0.0f};
vec3  diffuse_c  = {0.0f, 0.0f, 0.0f};
vec3  specular_c = {0.0f, 0.0f, 0.0f};

// temporary values for attenuation
float constant = 1.0f; 
float linear = 0.0022f;
float quadratic = 0.00019f;


void main()
{ 
    for(int i=0; i<3; i++)
    {
        vec3 L = normalize(lightSources[i].position.xyz-fragmentPosition);
        vec3 E = normalize(cameraPos-fragmentPosition);
        vec3 R = normalize(reflect(-L,N));
        float attenuation;
        float dist = length(lightPositions[i].xyz-fragmentPosition);

        // ball position vectors
        vec3 fragmentToLight = lightPositions[i].xyz - fragmentPosition;
        vec3 fragmentToBall = ballPos - fragmentPosition;

        if(length(reject(fragmentToBall, fragmentToLight)) > ballRad || length(fragmentToLight) < length(fragmentToBall)  || dot(fragmentToBall, fragmentToLight) < 0)
        {
            // attenuation
            attenuation = 1.0f/(lightSources[i].constant + (lightSources[i].linear*dist) + (lightSources[i].quadratic*dist*dist));

            // ambient color
            ambient_c += lightSources[i].ambient;

            // diffuse color
            diffuse_i = (max(dot(N,L), 0.0));
            diffuse_c += diffuse_i*lightSources[i].diffuse*attenuation;

            // specular color
            specular_i = pow(max(dot(R, E), 0.0), 32);
            specular_c += specular_i*lightSources[i].specular*attenuation;
        }
    }
    
    color = vec4((ambient_c + diffuse_c + specular_c), 1.0f) + dither(textureCoordinates);
}

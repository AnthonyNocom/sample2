#version 330 core
// POINT LIGHT ASSIGNMENT 04 - LIGHT TYPES

uniform sampler2D tex0;

uniform vec3 lightPos;
uniform vec3 lightColor;

uniform float ambientStr;
uniform vec3 ambientColor;

uniform vec3 cameraPos;
uniform float specStr;
uniform float specPhong;

in vec2 texCoord;
in vec3 normCoord;
in vec3 fragPos;

out vec4 Fragcolor;

void main()
{
    vec3 normal = normalize(normCoord);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 ambientCol = ambientColor * ambientStr;

    vec3 viewDir = normalize(cameraPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    float spec = pow(max(dot(reflectDir, viewDir), 0.1), specPhong);
    vec3 specColor = spec * specStr * lightColor;

    float distance = length(lightPos - fragPos);

    float attenuation = 1.0 / (1.0f +  0.0014f * distance + .000007f * (distance * distance));
    specColor *= attenuation;
    diffuse *= attenuation;
    ambientCol *= attenuation;

    Fragcolor = vec4(specColor + diffuse + ambientCol, 1.0) * texture(tex0, texCoord);
}
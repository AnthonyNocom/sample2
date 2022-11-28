#version 330 core

uniform sampler2D tex0;

uniform sampler2D norm_tex;

uniform sampler2D tex2; // for rendering yae.png onto the plane

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

in mat3 TBN;

out vec4 Fragcolor;

void main()
{
    vec4 pixelColor = texture(tex0, texCoord);

    if(pixelColor.a < 0.1) {
        discard;
        // below here is ignored
    }

    // gets rgb data of texture
    vec3 normal = texture(norm_tex, texCoord).rgb;
    // converts RGB -> XYZ; 0 == -1 ; 1 == 1
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(TBN * normal);

    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 ambientCol = ambientColor * ambientStr;

    vec3 viewDir = normalize(cameraPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    float spec = pow(max(dot(reflectDir, viewDir), 0.1), specPhong);
    vec3 specColor = spec * specStr * lightColor;

    float distance = length(lightPos - fragPos);

    float attenuation = 1.0f / (1.0f +  0.0014f * distance + .000007f * (distance * distance));
    specColor *= attenuation;
    diffuse *= attenuation;
    ambientCol *= attenuation;

    vec4 brick_wall = vec4(1.0f, 1.0f, 1.0f, 0.6f) * texture(tex0, texCoord); // we make the brick wall texture 0.6 opacity by changing alpha 
    vec4 mixColor = mix(brick_wall, texture(tex2, texCoord), texture(tex2, texCoord).a); // mix the transparent brick wall with yae.png based on alpha of yae.png
    Fragcolor = vec4(specColor + diffuse + ambientCol, 1.0) * mixColor;
}
// code taken from learnopengl.com

#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Default light values
const glm::vec3 LIGHTPOS = glm::vec3(0, 30, 0);
const glm::vec3 LIGHTCOLOR = glm::vec3(1, 1, 1);
const float AMBIENTSTR = 0.75f;
const float SPECSTR = 0.5f;
const float SPECPHONG = 16.0f;

class Light
{
public:
    // light attributes
    glm::vec3 lightPos;
    glm::vec3 lightColor;
    // ambient
    float ambientStr;
    glm::vec3 ambientColor;
    // specular
    float specStr;
    float specPhong;

    // constructor with vectors
    Light()
    {
        lightPos = LIGHTPOS;
        lightColor = LIGHTCOLOR;
        ambientStr = AMBIENTSTR;
        ambientColor = LIGHTCOLOR;
        specStr = SPECSTR;
        specPhong = SPECPHONG;
    }

    Light(glm::vec3 lightPosition)
    {
        lightPos = lightPosition;
        lightColor = LIGHTCOLOR;
        ambientStr = AMBIENTSTR;
        ambientColor = LIGHTCOLOR;
        specStr = SPECSTR;
        specPhong = SPECPHONG;
    }

    void updateLightPos(glm::vec3 lightPosition)
    {
        lightPos = lightPosition;
    }

    void updateLightStr(float ambientStrength)
    {
        specStr = ambientStrength;
    }
};
//class DirectionLight :
//    public Light
//{
//    
//};
//
//class PointLight :
//    public Light
//{
//    
//};
#endif
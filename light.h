// code taken from learnopengl.com

#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Default light values
const glm::vec3 LIGHTCOLOR = glm::vec3(1, 1, 1);
const glm::vec3 AMBIENT = glm::vec3(0.35f, 0.35f, 0.35f);
const glm::vec3 DIFFUSE = glm::vec3(0.4f, 0.4f, 0.4f);
const glm::vec3 SPECULAR = glm::vec3(0.5f, 0.5f, 0.5f);

class Light
{
public:
    // light attributes
    glm::vec3 color;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    // constructor with vectors
    Light()
    {
        color = LIGHTCOLOR;
        ambient = AMBIENT;
        diffuse = DIFFUSE;
        specular = SPECULAR;
    }

    // use this when cycling **SPOTLIGHT** intensity. 
    // ambientStrength cycle values: 0.35f, 0.50f, or 0.65f ?
    void updateLightStr(float ambientStrength)
    {
        ambient = glm::vec3(ambientStrength, ambientStrength, ambientStrength);
    }
};

// Automatically y = -1000.0f to look like its from the sky/ocean surface
class DirectionLight : public Light
{
public:
    glm::vec3 direction;

    DirectionLight()
    {
        direction = glm::vec3(0.0f, -1000.0f, 0.0f);
        ambient = AMBIENT;
        diffuse = DIFFUSE;
        specular = SPECULAR;
    }
};


class SpotLight : public Light
{
public:
    glm::vec3 position;
    glm::vec3 direction;

    // attenuation
    float constant;
    float linear;
    float quadratic;

    // spot light stuff
    float cutOff;
    float outerCutOff;

    // use cameraPos for position and cameraFront for direction
    SpotLight(glm::vec3 cameraPos, glm::vec3 cameraFront)
    {
        position = cameraPos;
        direction = cameraFront;

        ambient = glm::vec3(0.0f, 0.0f, 0.0f);
        diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
        specular = glm::vec3(1.0f, 1.0f, 1.0f);

        constant = 1.0f;
        linear = 0.09f;
        quadratic = 0.032f;

        cutOff = glm::cos(glm::radians(12.5f));
        outerCutOff = glm::cos(glm::radians(15.0f));
    }
};
#endif
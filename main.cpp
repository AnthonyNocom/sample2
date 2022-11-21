#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <string>
#include <iostream>

#include "Model3D.h"
#include "camera.h" // camera
#include "light.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);

// settings
float screenWidth = 750.f;
float screenHeight = 750.f;
float speed = 2.5f;

// object rotation
float theta_x = 0.0f;
float theta_y = 0.0f;
float theta_z = 0.0f;
float theta_x2 = 0.0f;
float theta_y2 = 0.0f;
float theta_z2 = 0.0f;

// object being controlled (main or light source)
bool objectControl = 0;
// camera (perspective or ortho)
bool cameraControl = 0;



// camera
OrthoCamera orthoCam(glm::vec3(0.0f, 10.f, 10.0f));
PerspectiveCamera persCam(glm::vec3(0.0f, 0.0f, 10.0f));

float lastX = screenWidth / 2.0f;
float lastY = screenHeight / 2.0f;
bool firstMouse = true;

glm::vec4 vec(1.0f, 1.0f, 1.0f, 1.0f);

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
Light light;
float pointLightStr = 0.5f;
float dirLightStr;

std::vector<Model3D> models;

glm::mat4 projection_matrix;
glm::mat4 view_matrix;

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(screenWidth, screenHeight, "Anthony Nocom", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();

    //glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // shader main
    std::fstream vertSrc("Shaders/sample.vert");
    std::stringstream vertBuff;
    vertBuff << vertSrc.rdbuf();
    std::string vertS = vertBuff.str();
    const char* v = vertS.c_str();

    std::fstream fragSrc("Shaders/sample.frag");
    std::stringstream fragBuff;
    fragBuff << fragSrc.rdbuf();
    std::string fragS = fragBuff.str();
    const char* f = fragS.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &v, NULL);
    glCompileShader(vertexShader);

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &f, NULL);
    glCompileShader(fragShader);

    GLuint shaderProg = glCreateProgram();
    glAttachShader(shaderProg, vertexShader);
    glAttachShader(shaderProg, fragShader);
    glLinkProgram(shaderProg);


    stbi_set_flip_vertically_on_load(true);

    // Texture 1
    int img_width, img_height, color_channels;

    unsigned char* tex_bytes = stbi_load("3D/Marble016_1K_Color.jpg",
        &img_width,
        &img_height,
        &color_channels,
        0);

    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        img_width,
        img_height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        tex_bytes
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(tex_bytes);

    // object 1, UV
    std::string path = "3D/Objects/Gun.obj";
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> material;
    std::string warning, error;
    tinyobj::attrib_t attributes;
    bool success = tinyobj::LoadObj(&attributes,
        &shapes,
        &material,
        &warning,
        &error,
        path.c_str());
    std::vector<GLuint> mesh_indices;
    for (int i = 0; i < shapes[0].mesh.indices.size(); i++) {
        mesh_indices.push_back(
            shapes[0].mesh.indices[i].vertex_index
        );
    }
    std::vector <GLfloat> fullVertexData;
    for (int i = 0; i < shapes[0].mesh.indices.size(); i++) {
        tinyobj::index_t vData = shapes[0].mesh.indices[i];

        fullVertexData.push_back(
            attributes.vertices[(vData.vertex_index * 3)]
        );
        fullVertexData.push_back(
            attributes.vertices[(vData.vertex_index * 3) + 1]
        );
        fullVertexData.push_back(
            attributes.vertices[(vData.vertex_index * 3) + 2]
        );
        
        fullVertexData.push_back(
            attributes.normals[(vData.normal_index * 3)]
        );
        fullVertexData.push_back(
            attributes.normals[(vData.normal_index * 3) + 1]
        );
        fullVertexData.push_back(
            attributes.normals[(vData.normal_index * 3) + 2]
        );

        fullVertexData.push_back(
            attributes.texcoords[(vData.texcoord_index * 2)]
        );
        fullVertexData.push_back(
            attributes.texcoords[(vData.texcoord_index * 2) + 1]
        );
    }

    // object 2, normal
    std::string path2 = "3D/djSword.obj";
    std::vector<tinyobj::shape_t> shapes2;
    std::vector<tinyobj::material_t> material2;
    std::string warning2, error2;
    tinyobj::attrib_t attributes2;
    bool success2 = tinyobj::LoadObj(&attributes2,
        &shapes2,
        &material2,
        &warning2,
        &error2,
        path2.c_str());
    std::vector<GLuint> mesh_indices2;
    for (int i = 0; i < shapes2[0].mesh.indices.size(); i++) {
        mesh_indices2.push_back(
            shapes2[0].mesh.indices[i].vertex_index
        );
    }
    std::vector <GLfloat> fullVertexData2;
    for (int i = 0; i < shapes2[0].mesh.indices.size(); i++) {
        tinyobj::index_t vData2 = shapes2[0].mesh.indices[i];

        fullVertexData2.push_back(
            attributes2.vertices[(vData2.vertex_index * 3)]
        );
        fullVertexData2.push_back(
            attributes2.vertices[(vData2.vertex_index * 3) + 1]
        );
        fullVertexData2.push_back(
            attributes2.vertices[(vData2.vertex_index * 3) + 2]
        );

        fullVertexData2.push_back(
            attributes2.normals[(vData2.normal_index * 3)]
        );
        fullVertexData2.push_back(
            attributes2.normals[(vData2.normal_index * 3) + 1]
        );
        fullVertexData2.push_back(
            attributes2.normals[(vData2.normal_index * 3) + 2]
        );
    }

    // VAO 1
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(GL_FLOAT)* fullVertexData.size(),
        fullVertexData.data(),
        GL_STATIC_DRAW
    );
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(GL_FLOAT),
        (void*)0
    );
    GLintptr normPtr = 3 * sizeof(float);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(GL_FLOAT),
        (void*)normPtr
    );
    GLintptr uvPtr = 6 * sizeof(float);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(GL_FLOAT),
        (void*)uvPtr
    );
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // VAO 2
    GLuint VAO2, VBO2;
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);
    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(GL_FLOAT)* fullVertexData2.size(),
        fullVertexData2.data(),
        GL_STATIC_DRAW
    );
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(GL_FLOAT),
        (void*)0
    );
    GLintptr normPtr2 = 3 * sizeof(float);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(GL_FLOAT),
        (void*)normPtr2
    );

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    projection_matrix = glm::perspective(
        glm::radians(60.0f),
        screenHeight / screenWidth,
        0.1f,
        100.f
    );

    glm::mat3 identity_matrix3 = glm::mat3(1.0f);
    glm::mat4 identity_matrix4 = glm::mat4(1.0f);

    float x, y, z;
    x = y = z = 0.0f;

    float scale_x, scale_y, scale_z;
    scale_x = scale_y = scale_z = 0.10f;

    float rot_x, rot_y, rot_z;
    rot_x = rot_y = rot_z = 0;
    rot_y = 1.0f;

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // object 1
        glm::mat4 transformation_matrix = glm::mat4(1.0f);
        if (!cameraControl) {
            view_matrix = persCam.lookAtOrigin();
        }
        else {
            view_matrix = orthoCam.lookFromAbove();
        }

        transformation_matrix = glm::translate(transformation_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
        transformation_matrix = glm::scale(transformation_matrix, glm::vec3(5.0f, 5.0f, 5.0f));
        transformation_matrix = glm::rotate(transformation_matrix, glm::radians(theta_x), glm::normalize(glm::vec3(1, 0, 0))); // spin clockwise
        transformation_matrix = glm::rotate(transformation_matrix, glm::radians(theta_y), glm::normalize(glm::vec3(0, 1, 0)));
        theta_y += 1.f;
        transformation_matrix = glm::rotate(transformation_matrix, glm::radians(theta_z), glm::normalize(glm::vec3(0, 0, 1)));

        GLuint tex0Address = glGetUniformLocation(shaderProg, "tex0");
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(tex0Address, 0);

        GLuint lightAddress = glGetUniformLocation(shaderProg, "lightPos");
        glUniform3fv(lightAddress, 1, glm::value_ptr(light.lightPos));

        GLuint lightColorAddress = glGetUniformLocation(shaderProg, "lightColor");
        glUniform3fv(lightColorAddress, 1, glm::value_ptr(light.lightColor));

        GLuint ambientStrAddress = glGetUniformLocation(shaderProg, "ambientStr");
        glUniform1f(ambientStrAddress, light.ambientStr);

        GLuint ambientColorAddress = glGetUniformLocation(shaderProg, "ambientColor");
        glUniform3fv(ambientColorAddress, 1, glm::value_ptr(light.ambientColor));

        
        GLuint specStrAddress = glGetUniformLocation(shaderProg, "specStr");
        glUniform1f(specStrAddress, light.specStr);

        GLuint specPhongAddress = glGetUniformLocation(shaderProg, "specPhong");
        glUniform1f(specPhongAddress, light.specPhong);

        unsigned int projectionLoc = glGetUniformLocation(shaderProg, "projection");
        glUniformMatrix4fv(projectionLoc,
            1,
            GL_FALSE,
            glm::value_ptr(projection_matrix));

        unsigned int viewLoc = glGetUniformLocation(shaderProg, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view_matrix));

        unsigned int transformLoc = glGetUniformLocation(shaderProg, "transform");
        glUniformMatrix4fv(transformLoc,
            1,
            GL_FALSE,
            glm::value_ptr(transformation_matrix));

        glUseProgram(shaderProg);

        glBindVertexArray(VAO);

        glDrawArrays(GL_TRIANGLES, 0, fullVertexData.size() / 8);

        // object 2 sword
        transformation_matrix = glm::mat4(1.0f);
        if (!cameraControl) {
            view_matrix = persCam.lookAtOrigin();
        }
        else {
            view_matrix = orthoCam.lookFromAbove();
        }

        transformation_matrix = glm::translate(transformation_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
        transformation_matrix = glm::rotate(transformation_matrix, glm::radians(theta_x2), glm::normalize(glm::vec3(1, 0, 0)));
        transformation_matrix = glm::rotate(transformation_matrix, glm::radians(theta_y2), glm::normalize(glm::vec3(0, 1, 0)));
        transformation_matrix = glm::rotate(transformation_matrix, glm::radians(theta_z2), glm::normalize(glm::vec3(0, 0, 1)));
        transformation_matrix = glm::translate(transformation_matrix, glm::vec3(3.f, 3.0f, 0.0f));
        transformation_matrix = glm::scale(transformation_matrix, glm::vec3(0.1f, 0.05f, 0.1f));

        vec = transformation_matrix * vec;
        light.updateLightPos(glm::vec3(vec.x, vec.y, vec.z)); // light source
        light.updateLightStr(pointLightStr);

        lightAddress = glGetUniformLocation(shaderProg, "lightPos");
        glUniform3fv(lightAddress, 1, glm::value_ptr(light.lightPos));

        lightColorAddress = glGetUniformLocation(shaderProg, "lightColor");
        glUniform3fv(lightColorAddress, 1, glm::value_ptr(light.lightColor));

        ambientStrAddress = glGetUniformLocation(shaderProg, "ambientStr");
        glUniform1f(ambientStrAddress, 1.f);

        glm::vec3 lightColor;
        if (!objectControl) {
            lightColor = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        else lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

        ambientColorAddress = glGetUniformLocation(shaderProg, "ambientColor");
        glUniform3fv(ambientColorAddress, 1, glm::value_ptr(lightColor));

        specStrAddress = glGetUniformLocation(shaderProg, "specStr");
        glUniform1f(specStrAddress, light.specStr);

        specPhongAddress = glGetUniformLocation(shaderProg, "specPhong");
        glUniform1f(specPhongAddress, light.specPhong);

        projectionLoc = glGetUniformLocation(shaderProg, "projection");
        glUniformMatrix4fv(projectionLoc,
            1,
            GL_FALSE,
            glm::value_ptr(projection_matrix));

        viewLoc = glGetUniformLocation(shaderProg, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view_matrix));

        transformLoc = glGetUniformLocation(shaderProg, "transform");
        glUniformMatrix4fv(transformLoc,
            1,
            GL_FALSE,
            glm::value_ptr(transformation_matrix));

        glUseProgram(shaderProg);

        glBindVertexArray(VAO2);

        glDrawArrays(GL_TRIANGLES, 0, fullVertexData2.size() / 6);
        // processInput(window, transformLoc);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }
    // delete buffers
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        if (objectControl == 0)
        {
            objectControl = 1;
        }
        else {
            objectControl = 0;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        // perspective camera
        cameraControl = 0;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        // ortho camera
        cameraControl = 1;
    }

    if (objectControl) { // light source 
        light.lightColor = glm::vec3(1, 1, 1);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            theta_x += speed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            theta_x -= speed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            theta_y += speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            theta_y -= speed;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            theta_z += speed;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            theta_z -= speed;
    }
    else {
        light.lightColor = glm::vec3(0, 1, 0);
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            theta_x2 += speed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            theta_x2 -= speed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            theta_y2 += speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            theta_y2 -= speed;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            theta_z2 += speed;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            theta_z2 -= speed;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        pointLightStr += .05f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        pointLightStr -= .05f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        dirLightStr += .05f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        dirLightStr -= .05f;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    persCam.ProcessMouseMovement(xoffset, yoffset);
}
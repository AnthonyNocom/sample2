#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <string>
#include <iostream>

#include "Model3D.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window, unsigned int transformLoc);

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -10.0f); // cameraCenter = cameraPos + cameraFront = 10.0f + (-10.0f) = 0,0,0
glm::vec3 cameraUp = glm::normalize(glm::vec3(0, 1.f, 0));

bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

std::vector<Model3D> models;

glm::mat4 projection_matrix;
glm::mat4 view_matrix;

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    float screenWidth = 750.f;
    float screenHeight = 750.f;

    window = glfwCreateWindow(screenWidth, screenHeight, "Anthony Nocom", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();

    //glfwSetCursorPosCallback(window, mouse_callback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
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

    unsigned char* tex_bytes = stbi_load("3D/Grass001_1K_Color.jpg",
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

    // texture 2
    int img_width2, img_height2, color_channels2;

    unsigned char* tex_bytes2 = stbi_load("3D/Marble016_1K_Color.jpg",
        &img_width,
        &img_height,
        &color_channels,
        0);
    GLuint texture2;
    glGenTextures(1, &texture2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        img_width,
        img_height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        tex_bytes2
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(tex_bytes2);

    // texture 3
    int img_width3, img_height3, color_channels3;

    unsigned char* tex_bytes3 = stbi_load("3D/Wood066_1K_Color.jpg",
        &img_width3,
        &img_height3,
        &color_channels3,
        0);
    GLuint texture3;
    glGenTextures(1, &texture3);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture3);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        img_width3,
        img_height3,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        tex_bytes3
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(tex_bytes3);
    glEnable(GL_DEPTH_TEST);

    // object 1, UV
    std::string path = "3D/teacup.obj";
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

    // object 3, normal + UV
    std::string path3 = "3D/spoon.obj";
    std::vector<tinyobj::shape_t> shapes3;
    std::vector<tinyobj::material_t> material3;
    std::string warning3, error3;
    tinyobj::attrib_t attributes3;
    bool success3 = tinyobj::LoadObj(&attributes3,
        &shapes3,
        &material3,
        &warning3,
        &error3,
        path3.c_str());
    std::vector<GLuint> mesh_indices3;
    for (int i = 0; i < shapes3[0].mesh.indices.size(); i++) {
        mesh_indices3.push_back(
            shapes3[0].mesh.indices[i].vertex_index
        );
    }
    std::vector <GLfloat> fullVertexData3;
    for (int i = 0; i < shapes3[0].mesh.indices.size(); i++) {
        tinyobj::index_t vData3 = shapes3[0].mesh.indices[i];

        fullVertexData3.push_back(
            attributes3.vertices[(vData3.vertex_index * 3)]
        );
        fullVertexData3.push_back(
            attributes3.vertices[(vData3.vertex_index * 3) + 1]
        );
        fullVertexData3.push_back(
            attributes3.vertices[(vData3.vertex_index * 3) + 2]
        );

        fullVertexData3.push_back(
            attributes3.normals[(vData3.normal_index * 3)]
        );
        fullVertexData3.push_back(
            attributes3.normals[(vData3.normal_index * 3) + 1]
        );
        fullVertexData3.push_back(
            attributes3.normals[(vData3.normal_index * 3) + 2]
        );

        fullVertexData3.push_back(
            attributes3.texcoords[(vData3.texcoord_index * 2)]
        );
        fullVertexData3.push_back(
            attributes3.texcoords[(vData3.texcoord_index * 2) + 1]
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
        5 * sizeof(GL_FLOAT),
        (void*)0
    );
    GLintptr uvPtr = 3 * sizeof(float);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(GL_FLOAT),
        (void*)uvPtr
    );
    glEnableVertexAttribArray(0);
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

    // VAO 3
    GLuint VAO3, VBO3;
    glGenVertexArrays(1, &VAO3);
    glGenBuffers(1, &VBO3);
    glBindVertexArray(VAO3);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(GL_FLOAT)* fullVertexData3.size(),
        fullVertexData3.data(),
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
    GLintptr normPtr3 = 3 * sizeof(float);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(GL_FLOAT),
        (void*)normPtr3
    );
    GLintptr uvPtr3 = 6 * sizeof(float);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(GL_FLOAT),
        (void*)uvPtr3
    );
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
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
    float theta_x = 0.0f;
    float theta_y = 0.0f;

    glm::vec3 lightPos = glm::vec3(10, 10, 0);
    glm::vec3 lightColor = glm::vec3(1, 1, 1);

    float ambientStr = 0.35f;
    glm::vec3 ambientColor = lightColor;

    float specStr = 0.5f;
    float specPhong = 16.0f;

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // object 1 teacup
        glm::mat4 transformation_matrix = glm::mat4(1.0f);
        view_matrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        transformation_matrix = glm::translate(transformation_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
        transformation_matrix = glm::scale(transformation_matrix, glm::vec3(2.0f, 2.0f, 2.0f));
        transformation_matrix = glm::rotate(transformation_matrix, glm::radians(-60.0f), glm::normalize(glm::vec3(rot_x, rot_y, rot_z)));

        GLuint tex0Address = glGetUniformLocation(shaderProg, "tex0");
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(tex0Address, 0);

        GLuint lightAddress = glGetUniformLocation(shaderProg, "lightPos");
        glUniform3fv(lightAddress, 1, glm::value_ptr(lightPos));

        GLuint lightColorAddress = glGetUniformLocation(shaderProg, "lightColor");
        glUniform3fv(lightColorAddress, 1, glm::value_ptr(lightColor));

        GLuint ambientStrAddress = glGetUniformLocation(shaderProg, "ambientStr");
        glUniform1f(ambientStrAddress, ambientStr);

        GLuint ambientColorAddress = glGetUniformLocation(shaderProg, "ambientColor");
        glUniform3fv(ambientColorAddress, 1, glm::value_ptr(ambientColor));

        GLuint cameraPosAddress = glGetUniformLocation(shaderProg, "cameraPos");
        glUniform3fv(cameraPosAddress, 1, glm::value_ptr(cameraPos));

        GLuint specStrAddress = glGetUniformLocation(shaderProg, "specStr");
        glUniform1f(specStrAddress, specStr);

        GLuint specPhongAddress = glGetUniformLocation(shaderProg, "specPhong");
        glUniform1f(specPhongAddress, specPhong);

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

        glDrawArrays(GL_TRIANGLES, 0, fullVertexData.size() / 5);

        // object 2 sword
        transformation_matrix = glm::mat4(1.0f);
        view_matrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        transformation_matrix = glm::translate(transformation_matrix, glm::vec3(3.5f, -1.0f, 0.0f));
        transformation_matrix = glm::scale(transformation_matrix, glm::vec3(0.1f, 0.1f, 0.1f));
        transformation_matrix = glm::rotate(transformation_matrix, glm::radians(-45.f), glm::normalize(glm::vec3(1, rot_y, rot_z)));

        tex0Address = glGetUniformLocation(shaderProg, "tex0");
        glBindTexture(GL_TEXTURE_2D, texture2);
        glUniform1i(tex0Address, 0);

        lightAddress = glGetUniformLocation(shaderProg, "lightPos");
        glUniform3fv(lightAddress, 1, glm::value_ptr(lightPos));

        lightColorAddress = glGetUniformLocation(shaderProg, "lightColor");
        glUniform3fv(lightColorAddress, 1, glm::value_ptr(lightColor));

        ambientStrAddress = glGetUniformLocation(shaderProg, "ambientStr");
        glUniform1f(ambientStrAddress, ambientStr);

        ambientColorAddress = glGetUniformLocation(shaderProg, "ambientColor");
        glUniform3fv(ambientColorAddress, 1, glm::value_ptr(ambientColor));

        cameraPosAddress = glGetUniformLocation(shaderProg, "cameraPos");
        glUniform3fv(cameraPosAddress, 1, glm::value_ptr(cameraPos));

        specStrAddress = glGetUniformLocation(shaderProg, "specStr");
        glUniform1f(specStrAddress, specStr);

        specPhongAddress = glGetUniformLocation(shaderProg, "specPhong");
        glUniform1f(specPhongAddress, specPhong);

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

        // object 3 spoon
        transformation_matrix = glm::mat4(1.0f);
        view_matrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        transformation_matrix = glm::translate(transformation_matrix, glm::vec3(-2.75f, 2.0f, 0.0f));
        transformation_matrix = glm::scale(transformation_matrix, glm::vec3(2.0f, 2.0f, 2.0f));
        transformation_matrix = glm::rotate(transformation_matrix, glm::radians(-45.0f), glm::normalize(glm::vec3(1, rot_y, rot_z)));

        tex0Address = glGetUniformLocation(shaderProg, "tex0");
        glBindTexture(GL_TEXTURE_2D, texture3);
        glUniform1i(tex0Address, 0);

        lightAddress = glGetUniformLocation(shaderProg, "lightPos");
        glUniform3fv(lightAddress, 1, glm::value_ptr(lightPos));

        lightColorAddress = glGetUniformLocation(shaderProg, "lightColor");
        glUniform3fv(lightColorAddress, 1, glm::value_ptr(lightColor));

        ambientStrAddress = glGetUniformLocation(shaderProg, "ambientStr");
        glUniform1f(ambientStrAddress, ambientStr);

        ambientColorAddress = glGetUniformLocation(shaderProg, "ambientColor");
        glUniform3fv(ambientColorAddress, 1, glm::value_ptr(ambientColor));

        cameraPosAddress = glGetUniformLocation(shaderProg, "cameraPos");
        glUniform3fv(cameraPosAddress, 1, glm::value_ptr(cameraPos));

        specStrAddress = glGetUniformLocation(shaderProg, "specStr");
        glUniform1f(specStrAddress, specStr);

        specPhongAddress = glGetUniformLocation(shaderProg, "specPhong");
        glUniform1f(specPhongAddress, specPhong);

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

        glBindVertexArray(VAO3);

        glDrawArrays(GL_TRIANGLES, 0, fullVertexData3.size() / 8);
        // processInput(window, transformLoc);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }
    // delete buffers
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);
    glDeleteVertexArrays(1, &VAO3);
    glDeleteBuffers(1, &VBO3);

    glfwTerminate();
    return 0;
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

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}


// for camera movement, taken from learnopengl.com
void processInput(GLFWwindow* window, unsigned int transformLoc)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(2.5 * deltaTime);
    /*if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (glfwGetTime() >= 3) {
            models.push_back(Model3D(cameraPos, mesh_indices.size(), cameraFront));
            glfwSetTime(0.0);
        }
    }
    else {*/
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    //}
};
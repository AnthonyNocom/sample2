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

std::vector<GLuint> mesh_indices;
glm::mat4 projection_matrix;
glm::mat4 view_matrix;

// for camera movement, taken from learnopengl.com
void processInput(GLFWwindow* window, unsigned int transformLoc)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(2.5 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (glfwGetTime() >= 3) {
            models.push_back(Model3D(cameraPos, mesh_indices.size(), cameraFront));
            glfwSetTime(0.0);
        }
    }
    else { 
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
};

std::vector <GLfloat> load(const char* filename)
{
    std::string path = filename;
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

    /*for (int i = 0; i < shapes[0].mesh.indices.size(); i++) {
        mesh_indices.push_back(shapes[0].mesh.indices[i].vertex_index);
    }*/

    // loop through all the vertex indices
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
    return fullVertexData;
}

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

    GLfloat vertices[]{
    -0.5f,-0.5f,0,
    0,0.5f,0,
    0.5,-0.5f,0
    };

    GLuint indices[] = {
        0,1,2
    };

    // GRAPHIX 07
    GLfloat UV[]{
        0.f, 1.f,
        0.f, 0.f,
        1.f, 1.f,
        1.f, 0.f,
        1.f, 1.f,
        1.f, 0.f,
        0.f, 1.f,
        0.f, 0.f
    };

    // GRAPHIX 07
    // ------------------------------
    stbi_set_flip_vertically_on_load(true);
    int img_width, img_height, color_channels;
    unsigned char* tex_bytes[2];
    tex_bytes[0] = stbi_load("3D/Grass001_1K_Color.jpg",
        &img_width,
        &img_height,
        &color_channels, 
        0);
    tex_bytes[1] = stbi_load("3D/Marble016_1K_Color.jpg",
        &img_width,
        &img_height,
        &color_channels,
        0);
    tex_bytes[2] = stbi_load("3D/Wood066_1K_Color.jpg",
        &img_width,
        &img_height,
        &color_channels,
        0);

    // reference to texture
    GLuint texture;
    //generate reference
    glGenTextures(1, &texture);
    // set texture we are working on to texture 0 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // assign the loaded texture to the reference
    glTexImage2D(
        GL_TEXTURE_2D,
        0,  // texture 0
        GL_RGB, // target color format
        img_width,
        img_height,
        0,
        GL_RGB,    // color format
        GL_UNSIGNED_BYTE,
        tex_bytes[0]   // loaded texture in bytes
    );

    // generate mipmaps to the current texture
    glGenerateMipmap(GL_TEXTURE_2D);
    // free up the loaded bytes
    stbi_image_free(tex_bytes[0]);
    glEnable(GL_DEPTH_TEST);
    // ------------------------------

    std::vector<GLfloat> fullVertexData = load("3D/djSword.obj");

    GLuint VAO, VBO, EBO, VBO_UV;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    //glGenBuffers(1, &VBO_UV); // remove code graphix 08
    //glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // add in our new array of vertex data in the VBO
    glBufferData(
        GL_ARRAY_BUFFER,
        //size of the whole array in bytes
        sizeof(GLfloat)* fullVertexData.size(),
        //data of array
        fullVertexData.data(),
        GL_DYNAMIC_DRAW
    );

    // add in how to get the position data from our array
    glVertexAttribPointer(
        0, // index 0 is vertex position
        3, // position is 3 floats (X,Y,Z)
        GL_FLOAT, // data type of array
        GL_FALSE,
        // our vertex data has 5 floats in it
        // (X,Y,Z,U,V)
        8 * sizeof(float), // size of vertex data in bytes ----- it is now 8
        (void*)0
    );

    GLintptr normPtr = 3 * sizeof(float); // index 4, 5, 6
    GLintptr uvPtr = 6 * sizeof(float); // UV now starts at 6? index 7, 8

    // ASSIGNMENT 3 - VERTEX ATTRIBUTES
    // ----------------------------------
    // insert the normals data
    glVertexAttribPointer(
        1, // index 1 is normals
        3, // normal is 3 floats
        GL_FLOAT, // data type
        GL_FALSE,
        8 * sizeof(float),
        // add in the offset
        (void*)normPtr
    );
    // ----------------------------------
    // add in how to get the uv data from our array
    glVertexAttribPointer(
        2, // index 2 is tex coordinates / UV
        2, // UV is 2 floats (U,V)
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        // add in the offset
        (void*)uvPtr
    );
    /*glBufferData(GL_ARRAY_BUFFER,
        sizeof(GL_FLOAT) * attributes.vertices.size(),
        &attributes.vertices[0],
        GL_STATIC_DRAW);

    glVertexAttribPointer(0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(GL_FLOAT),
        (void*)0);*/

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER,
    //    sizeof(GLuint) * mesh_indices.size(),
    //    mesh_indices.data(),
    //    GL_STATIC_DRAW);
    //// GRAPHIX 07
    //glBindBuffer(GL_ARRAY_BUFFER, VBO_UV);
    //glBufferData(GL_ARRAY_BUFFER,
    //    // size of float * length of array
    //    sizeof(GLfloat) * (sizeof(UV) / sizeof(UV[0])),
    //    &UV[0],
    //    GL_STATIC_DRAW); // or GL_DYNAMIC_DRAW

    //glVertexAttribPointer(
    //    2,  //Tex Coords / UV = index 2
    //    2,  //U and V
    //    GL_FLOAT,
    //    GL_FALSE,
    //    2 * sizeof(GL_FLOAT),
    //    (void*)0
    //);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
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
    glm::mat4 translation =
        glm::translate(identity_matrix4,
            glm::vec3(x, y, z)
        );

    float scale_x, scale_y, scale_z;
    scale_x = scale_y = scale_z = 0.10f;
    glm::mat4 scale =
        glm::scale(identity_matrix4,
            glm::vec3(scale_x, scale_y, scale_z)
        );

    float rot_x, rot_y, rot_z;
    rot_x = rot_y = rot_z = 0;
    rot_y = 1.0f;
    float theta_x = 0.0f;
    float theta_y = 0.0f;
    glm::mat4 rotation =
        glm::rotate(identity_matrix4,
            glm::radians(theta_x),
            glm::normalize(glm::vec3(rot_x, rot_y, rot_z))
        );

    // append first model 
    //models.push_back(Model3D(cameraPos, mesh_indices.size(), cameraFront));

    glm::vec3 lightPos = glm::vec3(10, 10, 0);
    glm::vec3 lightColor = glm::vec3(1, 1, 1);

    float ambientStr = 0.5f;
    glm::vec3 ambientColor = lightColor;

    float specStr = 0.5f;
    float specPhong = 16;

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glm::mat4 transformation_matrix = glm::mat4(1.0f);
        transformation_matrix = glm::translate(transformation_matrix,
            glm::vec3(x, y, z));
        transformation_matrix = glm::scale(transformation_matrix,
            glm::vec3(scale_x, scale_y, scale_z));
        transformation_matrix = glm::rotate(transformation_matrix,
            glm::radians(theta_x),
            glm::normalize(glm::vec3(rot_x, rot_y, rot_z))
        );
        theta_x += 0.1;

        view_matrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        unsigned int transformLoc = glGetUniformLocation(shaderProg, "transform");
        glUniformMatrix4fv(transformLoc,
            1,
            GL_FALSE,
            glm::value_ptr(transformation_matrix));

        // bind texture
        GLuint tex0Address = glGetUniformLocation(shaderProg, "tex0");
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(tex0Address, 0);

        // GRAPHIX 09
        // get the address of the light position from the shader
        GLuint lightAddress = glGetUniformLocation(shaderProg, "lightPos");
        glUniform3fv(lightAddress, 1, glm::value_ptr(lightPos));

        // get the address of the light color from the shader
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
   
        glUseProgram(shaderProg);

        glBindVertexArray(VAO);

        // Instead of using the EBO, we render using the vertex array
        //glDrawElements(GL_TRIANGLES, mesh_indices.size(), GL_UNSIGNED_INT, 0);
        glDrawArrays(GL_TRIANGLES, 0, fullVertexData.size() / 8);

        processInput(window, transformLoc);

        //// Draw all models
        //for (auto i : models) {
        //    i.draw(transformLoc);
        //}
        glfwSwapBuffers(window);

        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
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
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
float pointLightStr = 0.1f;
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

    // shader skybox
    std::fstream skyboxVertSrc("Shaders/skybox.vert");
    std::stringstream skyboxVertBuff;
    skyboxVertBuff << skyboxVertSrc.rdbuf();
    std::string skyboxVertS = skyboxVertBuff.str();
    const char* sky_v = skyboxVertS.c_str();

    std::fstream skyboxFragSrc("Shaders/skybox.frag");
    std::stringstream skyboxFragBuff;
    skyboxFragBuff << skyboxFragSrc.rdbuf();
    std::string skyboxFragS = skyboxFragBuff.str();
    const char* sky_f = skyboxFragS.c_str();

    GLuint vertexShaderSkybox = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderSkybox, 1, &sky_v, NULL);
    glCompileShader(vertexShaderSkybox);

    GLuint fragShaderSkybox = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShaderSkybox, 1, &sky_f, NULL);
    glCompileShader(fragShaderSkybox);

    GLuint skyboxShaderProg = glCreateProgram();
    glAttachShader(skyboxShaderProg, vertexShaderSkybox);
    glAttachShader(skyboxShaderProg, fragShaderSkybox);
    glLinkProgram(skyboxShaderProg);

    glDeleteShader(vertexShaderSkybox);
    glDeleteShader(fragShaderSkybox);

    // Texture 1
    int img_width, img_height, color_channels;

    unsigned char* tex_bytes = stbi_load("3D/12/gradient.png",
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
        GL_RGBA,    //yae uses alpha
        img_width,
        img_height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        tex_bytes
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(tex_bytes);

    // object 1, UV
    std::string path = "3D/12/plane.obj";
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

        /*
      7--------6
     /|       /|
    4--------5 |
    | |      | |
    | 3------|-2
    |/       |/
    0--------1
    */
    //Vertices for the cube
    float skyboxVertices[]{
        -1.f, -1.f, 1.f, //0
        1.f, -1.f, 1.f,  //1
        1.f, -1.f, -1.f, //2
        -1.f, -1.f, -1.f,//3
        -1.f, 1.f, 1.f,  //4
        1.f, 1.f, 1.f,   //5
        1.f, 1.f, -1.f,  //6
        -1.f, 1.f, -1.f  //7
    };

    //Skybox Indices
    unsigned int skyboxIndices[]{
        1,2,6,
        6,5,1,

        0,4,7,
        7,3,0,

        4,5,6,
        6,7,4,

        0,3,2,
        2,1,0,

        0,1,5,
        5,4,0,

        3,7,6,
        6,2,3
    };

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
    
    // skybox VAO VBO EBO
    unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    // add in skybox vertices to buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    // vertices only take xyz so only 3
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // skybox ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GL_INT) * 36, &skyboxIndices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);

    std::string facesSkybox[]{
        "Skybox/rainbow_rt.png", //right
        "Skybox/rainbow_lf.png", //left
        "Skybox/rainbow_up.png", //up
        "Skybox/rainbow_dn.png", //down
        "Skybox/rainbow_ft.png", //front
        "Skybox/rainbow_bk.png", //back
    };

    // skybox tex
    unsigned int skyboxTex;
    //generate skybox textures
    glGenTextures(1, &skyboxTex);
    //initialize as cube map
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
    // this is to avoid skybox from pixelating when too big or small
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // instead of st only- cubemaps rely on RST
    // this is to make sure texture stretched to the edge
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    for (unsigned int i = 0; i < 6; i++) {
        // declare the width height and color channel of our skybox textures for later
        int w, h, skyCChannel;
        //temporarily disable flipping image
        stbi_set_flip_vertically_on_load(false);
        //load image as usual
        unsigned char* data = stbi_load(facesSkybox[i].c_str(), &w, &h, &skyCChannel, 0);

        if (data) {
            glTexImage2D(
                //cubemap face address start at positive_X (Right)
                //incrementing by 1
                //right > left > top > bot > front > back
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                GL_RGB, // some pngs dont have alpha
                w, // width
                h, // height
                0,
                GL_RGB,
                GL_UNSIGNED_BYTE, // data type
                data // texture data itself
            );
            //cleanup
            stbi_image_free(data);
        }
    }
    stbi_set_flip_vertically_on_load(true);
    

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
    scale_x = scale_y = scale_z = 5.f;

    float rot_x, rot_y, rot_z;
    rot_x = rot_y = rot_z = 0;
    rot_z = 1.0f;
    theta_z = -90.f;
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA);
        glBlendEquation(GL_FUNC_SUBTRACT);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //processInput(window);

        //disable depth mask
        glDepthMask(GL_FALSE);
        //change depth function to <=
        glDepthFunc(GL_LEQUAL);
        //use skybox shaders
        glUseProgram(skyboxShaderProg);

        // object 1
        glm::mat4 transformation_matrix = glm::mat4(1.0f);
        if (!cameraControl) {
            view_matrix = persCam.lookAtOrigin();
        }
        else {
            view_matrix = orthoCam.lookFromAbove();
        }

        // initialize skybox's view matrix
        glm::mat4 sky_view = glm::mat4(1.f);
        sky_view = glm::mat4(
            // cast same view matrix of camera
            //turn it into mat3 to remove translations
            glm::mat3(view_matrix)
            // reconvert to mat4
        );

        unsigned int skyboxViewLoc = glGetUniformLocation(skyboxShaderProg, "view");
        glUniformMatrix4fv(skyboxViewLoc, 1, GL_FALSE, glm::value_ptr(sky_view));

        unsigned int skyboxProjLoc = glGetUniformLocation(skyboxShaderProg, "projection");
        glUniformMatrix4fv(skyboxProjLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        glUseProgram(shaderProg);

        transformation_matrix = glm::translate(transformation_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
        transformation_matrix = glm::scale(transformation_matrix, glm::vec3(5.0f, 5.0f, 5.0f));
        transformation_matrix = glm::rotate(transformation_matrix, glm::radians(theta_x), glm::normalize(glm::vec3(1, 0, 0))); // spin clockwise
        
        transformation_matrix = glm::rotate(transformation_matrix, glm::radians(theta_y), glm::normalize(glm::vec3(0, 1, 0)));
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

        // processInput(window, transformLoc);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }
    // delete buffers
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

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
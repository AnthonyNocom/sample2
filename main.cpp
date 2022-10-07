#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <string>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float mod_z = 0;
float mod_x = 0;
float mod_y = 0;
float scale_mod = 1.0f;
float rot_mod_x = 0;
float rot_mod_y = 0;
float rot_mod_axis_x = 0; // rotate on x (0) or y (1) axis
float rot_mod_axis_y = 1; // rotate on x (0) or y (1) axis

void Key_Callback(GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods) {
    if (key == GLFW_KEY_D &&
        (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        mod_x += 0.2f;
    }

    if (key == GLFW_KEY_A &&
        (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        mod_x -= 0.2f;
    }

    if (key == GLFW_KEY_W &&
        (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        mod_y -= 0.2f;
    }

    if (key == GLFW_KEY_S &&
        (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        mod_y += 0.2f;
    }

    // Scale
    if (key == GLFW_KEY_E &&
        (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        scale_mod += 1.0f;
    }
    
    if (key == GLFW_KEY_Q &&
        (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        scale_mod -= 1.0f;
    }

    // Rotation
    if (key == GLFW_KEY_RIGHT &&
        (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        rot_mod_axis_x = 0;
        rot_mod_axis_y = 1;
        rot_mod_x += 30.0f;
    }

    if (key == GLFW_KEY_LEFT &&
        (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        rot_mod_axis_x = 0;
        rot_mod_axis_y = 1;
        rot_mod_x -= 30.0f;
    }

    if (key == GLFW_KEY_UP &&
        (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        rot_mod_axis_x = 1;
        rot_mod_axis_y = 0;
        rot_mod_y += 30.0f;
    }

    if (key == GLFW_KEY_DOWN &&
        (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        rot_mod_axis_x = 1; 
        rot_mod_axis_y = 0;
        rot_mod_y -= 30.0f;
    }
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    float screenWidth = 600.f;
    float screenHeight = 600.f;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(screenWidth, screenHeight, "Sample", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    gladLoadGL();

    glfwSetKeyCallback(window, Key_Callback);

    // Vertex Shader
    std::fstream vertSrc("Shaders/sample.vert");
    std::stringstream vertBuff;
    vertBuff << vertSrc.rdbuf();
    std::string vertS = vertBuff.str();
    const char* v = vertS.c_str();

    // Fragment Shader BLUE
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

    GLuint VAO, VBO, EBO;

    std::string path = "3D/bunny.obj";
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
        mesh_indices.push_back(shapes[0].mesh.indices[i].vertex_index);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER,
        sizeof(GL_FLOAT) * attributes.vertices.size(),
        &attributes.vertices[0],
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        sizeof(GLuint) * mesh_indices.size(),
        mesh_indices.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(GL_FLOAT),
        (void*)0);

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    glm::mat3 identity_matrix3 = glm::mat3(1.0f);
    glm::mat4 identity_matrix4 = glm::mat4(1.0f);

    float x, y, z;
    x = y = z = 0.0f;
    glm::mat4 translation =
        glm::translate(identity_matrix4,
            glm::vec3(x, y, z)
        );

    float scale_x, scale_y, scale_z;
    scale_x = scale_y = scale_z = 1.0f;
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

    z = -5.0f;

    
    glm::mat4 projection_matrix = glm::perspective(
        glm::radians(60.0f),
        screenHeight / screenWidth,
        0.1f,
        100.f
    );

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // -------------------------------------
        glm::vec3 cameraPos = glm::vec3(0, 0, 10.f);
        glm::mat4 cameraPosMatrix = glm::translate(glm::mat4(1.0f), cameraPos * -1.f);

        glm::vec3 WorldUp = glm::normalize(glm::vec3(0, 1.f, 0));
        glm::vec3 CameraCenter = glm::vec3(0, mod_y, 0);

        glm::vec3 F = CameraCenter - cameraPos;
        F = glm::normalize(F);

        glm::vec3 R = glm::cross(F, WorldUp);
        glm::vec3 U = glm::cross(R, F);

        glm::mat4 cameraOrientationMatrix = glm::mat4(1.0f);

        //1st Row
        cameraOrientationMatrix[0][0] = R.x;
        cameraOrientationMatrix[1][0] = R.y;
        cameraOrientationMatrix[2][0] = R.z;

        //2nd Row
        cameraOrientationMatrix[0][1] = U.x;
        cameraOrientationMatrix[1][1] = U.y;
        cameraOrientationMatrix[2][1] = U.z;

        //3rd Row
        cameraOrientationMatrix[0][2] = -F.x;
        cameraOrientationMatrix[1][2] = -F.y;
        cameraOrientationMatrix[2][2] = -F.z;

        //glm::mat4 viewMatrix = cameraOrientationMatrix * cameraPosMatrix;
        // ------------------------------------
        // this is equal to the lines of code encased above
        glm::mat4 viewMatrix = glm::lookAt(cameraPos, CameraCenter, WorldUp);

        // blue bunny
        glm::mat4 transformation_matrix = glm::mat4(1.0f);
        transformation_matrix = glm::translate(transformation_matrix,
            glm::vec3(0.6, 0.6, 0));
        transformation_matrix = glm::rotate(transformation_matrix,
            glm::radians(-90.f),
            glm::vec3(0, 1, rot_z));

        unsigned int transformLoc = glGetUniformLocation(shaderProg, "transform");
        glUniformMatrix4fv(transformLoc,
            1,
            GL_FALSE,
            glm::value_ptr(transformation_matrix));
       
        unsigned int projectionLoc = glGetUniformLocation(shaderProg, "projection");
        glUniformMatrix4fv(projectionLoc,
            1,
            GL_FALSE,
            glm::value_ptr(projection_matrix));

        unsigned int viewLoc = glGetUniformLocation(shaderProg, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));


        glUseProgram(shaderProg);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,
            mesh_indices.size(),
            GL_UNSIGNED_INT,
            0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
    return 0;
}
#include "Shader.h"
#include "Model.h"
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <matrix_transform.hpp>
#include <type_ptr.hpp>
#include <iostream>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct ObjectTransform {
    glm::vec3 position = glm::vec3(0.0f);


    // Ограничения движения
    struct {
        float minZ = -1.0f;
        float maxZ = 1.0f;
    } tableLimits; // Для X01 (движение по Z)

    struct {
        float minX = -5.0f;
        float maxX = 5.0f;
    } standLimits; // Для X02 (движение по X)

    struct {
        float minY = -1.0f;
        float maxY = 1.0f;
    } headLimits; // Для X03 (движение по Y)
};

std::vector<ObjectTransform> objectTransforms;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

glm::mat4 calculateModelMatrix(int index) {
    glm::mat4 model = glm::mat4(1.0f);

    if (index == 1) { // Base 
        return model;
    }

    if (index == 2) { // X01 
        model = glm::translate(model, objectTransforms[1].position);
        return model;
    }

    if (index == 3) { // X02 
        model = glm::translate(model, objectTransforms[1].position);
        model = glm::translate(model, objectTransforms[2].position); 
        return model;
    }

    if (index == 0) { // X03 
        model = glm::translate(model, objectTransforms[3].position);
        return model;
    }

    return model;
}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Model Transformations", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader shader("vertex_sheder.glsl", "fragment_shader.glsl");
    Model ourModel("model.obj");

    objectTransforms.resize(4);

    objectTransforms[1].tableLimits = { -0.9f, 1.0f }; // X01
    objectTransforms[2].standLimits = { -1.5f, 1.5f }; // X02
    objectTransforms[3].headLimits = { -1.0f, 0.5f };  // X03

    shader.use();
    shader.setVec3("light.position", glm::vec3(2.0f, 2.0f, 2.0f));
    shader.setVec3("light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
    shader.setVec3("light.diffuse", glm::vec3(0.1f, 0.1f, 0.1f));
    shader.setVec3("light.specular", glm::vec3(1.0f));
    shader.setVec3("material.ambient", glm::vec3(0.8f, 0.4f, 0.0f));
    shader.setVec3("material.diffuse", glm::vec3(0.9f, 0.5f, 0.1f));
    shader.setVec3("material.specular", glm::vec3(1.0f, 0.8f, 0.5f));
    shader.setFloat("material.shininess", 64.0f);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setVec3("viewPos", cameraPos);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        for (size_t i = 0; i < ourModel.meshTransforms.size(); ++i) {
            ourModel.meshTransforms[i] = calculateModelMatrix(i);
        }

        ourModel.Draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);


    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    float moveSpeed = 1.5f * deltaTime;

    // Управление X01
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        objectTransforms[1].position.z += moveSpeed;
        if (objectTransforms[1].position.z > objectTransforms[1].tableLimits.maxZ)
            objectTransforms[1].position.z = objectTransforms[1].tableLimits.maxZ;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        objectTransforms[1].position.z -= moveSpeed;
        if (objectTransforms[1].position.z < objectTransforms[1].tableLimits.minZ)
            objectTransforms[1].position.z = objectTransforms[1].tableLimits.minZ;
    }

    // Управление X02
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        objectTransforms[2].position.x += moveSpeed;
        if (objectTransforms[2].position.x > objectTransforms[2].standLimits.maxX)
            objectTransforms[2].position.x = objectTransforms[2].standLimits.maxX;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        objectTransforms[2].position.x -= moveSpeed;
        if (objectTransforms[2].position.x < objectTransforms[2].standLimits.minX)
            objectTransforms[2].position.x = objectTransforms[2].standLimits.minX;
    }

    // Управление X03 
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
        objectTransforms[3].position.y += moveSpeed;
        if (objectTransforms[3].position.y > objectTransforms[3].headLimits.maxY)
            objectTransforms[3].position.y = objectTransforms[3].headLimits.maxY;
    }
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
        objectTransforms[3].position.y -= moveSpeed;
        if (objectTransforms[3].position.y < objectTransforms[3].headLimits.minY)
            objectTransforms[3].position.y = objectTransforms[3].headLimits.minY;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
}

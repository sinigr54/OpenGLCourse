//
// Created by sinigr on 3/21/18.
//

#include <GLES3/gl3.h>
#include <utils/RenderUtils.h>
#include <utils/Random.h>
#include "Renderer.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <ctime>
#include <vector>
#include <sstream>
#include <application/renderer/camera/Camera.h>

const std::string TAG = "Rendered";

GLuint sceneShaderProgram = 0;
GLuint lightSourceShaderProgram = 0;

GLuint VBO = 0;
GLuint cubeVao = 0;
GLuint lightVao = 0;
GLuint IBO = 0;

GLfloat gVertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
};

float screenWidth = 0.0f;
float screenHeight = 0.0f;

const Camera camera(glm::vec3(-6.5f, 2.0f, -2.5f));

const glm::vec3 lightPosition(1.0f, 1.0f, 1.0f);

Renderer::Renderer(AAssetManager *assetManager) : assetManager(assetManager) {

}

void Renderer::onSurfaceCreated() {
    sceneShaderProgram = RenderUtils::createProgram(
            assetManager,
            "cube_vertex_shader.glsl",
            "cube_fragment_shader.glsl"
    );

    lightSourceShaderProgram = RenderUtils::createProgram(
            assetManager,
            "lamp_vertex_shader.glsl",
            "light_fragment_shader.glsl"
    );

    /*Создаем буфер верши и заполняем его вершинами*/
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gVertices), gVertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &lightVao);
    glBindVertexArray(lightVao);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *) 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    /*Создаем объект vertex array для куба и делаем ссылки на вершины*/
    glGenVertexArrays(1, &cubeVao);
    glBindVertexArray(cubeVao);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                          (GLvoid *) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

void Renderer::onSurfaceChanged(int width, int height) {
    glViewport(0, 0, width, height);
    screenWidth = static_cast<float>(width);
    screenHeight = static_cast<float>(height);
}

void Renderer::onDrawFrame() {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view;
    glm::mat4 projection;

    /* Создаем View матрицу камеры */
    view = camera.getViewMatrix();
    projection = glm::perspective(45.0f, screenWidth / screenHeight, 0.1f, 100.0f);

    glUseProgram(sceneShaderProgram);

    GLint modelLocation = glGetUniformLocation(sceneShaderProgram, "model");
    GLint viewLocation = glGetUniformLocation(sceneShaderProgram, "view");
    GLint projectionLocation = glGetUniformLocation(sceneShaderProgram, "projection");

    GLint objectColorLocation = glGetUniformLocation(sceneShaderProgram, "objectColor");
    GLint lightColorLocation = glGetUniformLocation(sceneShaderProgram, "lightColor");
    GLint lightPositionLocation = glGetUniformLocation(sceneShaderProgram, "lightPosition");
    GLint viewPositionLocation = glGetUniformLocation(sceneShaderProgram, "viewPosition");

    glm::mat4 cubeModel;

    glUniform3f(lightPositionLocation, lightPosition.x, lightPosition.y, lightPosition.z);
    glUniform3f(objectColorLocation, 1.0f, 0.5f, 0.31f);
    glUniform3f(lightColorLocation, 1.0f, 1.0f, 1.0f);
    glUniform3f(viewPositionLocation, camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);

    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(cubeModel));

    glBindVertexArray(cubeVao);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);

    glUseProgram(lightSourceShaderProgram);

    glm::mat4 lightModel;

    modelLocation = glGetUniformLocation(lightSourceShaderProgram, "model");
    viewLocation = glGetUniformLocation(lightSourceShaderProgram, "view");
    projectionLocation = glGetUniformLocation(lightSourceShaderProgram, "projection");

    lightModel = glm::translate(lightModel, lightPosition);
    lightModel = glm::scale(lightModel, glm::vec3(0.2f));

    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(lightModel));

    glBindVertexArray(lightVao);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);
}

//
// Created by Alexander Chekh on 15.05.2018.
//

#include "BackgroundRenderer.h"

const GLfloat gVertices[] = {
        -1.0f, -1.0f, 0.0f, +1.0f, -1.0f, 0.0f,
        -1.0f, +1.0f, 0.0f, +1.0f, +1.0f, 0.0f,
};

// UVs of the quad vertices (S, T)
const GLfloat gUvs[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
};

const GLfloat gCoordinates[]{
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        +1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, +1.0f, 0.0f, 0.0f, 0.0f,
        +1.0f, +1.0f, 0.0f, 1.0f, 0.0f
};

constexpr char gVertexShader[] = R"(
    attribute vec4 vertex;
    attribute vec2 textureCoords;
    varying vec2 v_textureCoords;
    void main() {
      v_textureCoords = textureCoords;
      gl_Position = vertex;
    })";

constexpr char gFragmentShader[] = R"(
    #extension GL_OES_EGL_image_external : require
    precision mediump float;
    uniform samplerExternalOES texture;
    varying vec2 v_textureCoords;
    void main() {
      gl_FragColor = texture2D(texture, v_textureCoords);
    })";

void BackgroundRenderer::initialize() {
    shaderProgram.init(gVertexShader, gFragmentShader);

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    attributeVertices = static_cast<GLuint>(glGetAttribLocation(shaderProgram.getProgram(),
                                                                "vertex"));
    attributeUvs = static_cast<GLuint>(glGetAttribLocation(shaderProgram.getProgram(),
                                                           "textureCoords"));
}

void BackgroundRenderer::draw(const ArSession *arSession, const ArFrame *arFrame) {
    static_assert(std::extent<decltype(gUvs)>::value == gNumVertices * 2,
                  "Incorrect gUvs length");

    static_assert(std::extent<decltype(gVertices)>::value == gNumVertices * 3,
                  "Incorrect gVertices length");

    // If display rotation changed (also includes view size change), we need to
    // re-query the uv coordinates for the on-screen portion of the camera image.
    int32_t isGeometryChanged = 0;
    ArFrame_getDisplayGeometryChanged(arSession, arFrame, &isGeometryChanged);
    if (isGeometryChanged != 0 || !isUvsInitialized) {

        ArFrame_transformDisplayUvCoords(arSession,
                                         arFrame,
                                         gNumVertices * 2,
                                         gUvs,
                                         transformedUvs);

        updateGraphicContext();

        isUvsInitialized = true;
    }

    shaderProgram.use();
    glDepthMask(GL_FALSE);

    shaderProgram.setUniform("texture", 1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);

    glBindVertexArray(VAO);

    /*glEnableVertexAttribArray(attributeVertices);
    glVertexAttribPointer(attributeVertices, 3, GL_FLOAT, GL_FALSE, 0,
                          gVertices);

    glEnableVertexAttribArray(attributeUvs);
    glVertexAttribPointer(attributeUvs, 2, GL_FLOAT, GL_FALSE, 0,
                          transformedUvs);*/

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);

    shaderProgram.unUse();
    glDepthMask(GL_TRUE);
}

GLuint BackgroundRenderer::getTextureId() const {
    return textureId;
}

void BackgroundRenderer::updateGraphicContext() {
    vertices.clear();

    for (auto i = 0; i < gNumVertices; ++i) {
        const auto iVertex = i * VERTEX_SIZE;
        const auto iCoords = i * COORDS_SIZE;

        vertices.emplace_back<Vertex>({
                                              glm::vec3(gVertices[iVertex],
                                                        gVertices[iVertex + 1],
                                                        gVertices[iVertex + 2]),
                                              glm::vec2(transformedUvs[iCoords],
                                                        transformedUvs[iCoords + 1])
                                      });
    }

    if (!isUvsInitialized) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 &vertices[0],
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(attributeVertices);
    glVertexAttribPointer(attributeVertices, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex),
                          (GLvoid *) 0);

    glEnableVertexAttribArray(attributeUvs);
    glVertexAttribPointer(attributeUvs, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex),
                          (GLvoid *) offsetof(Vertex, textureCoords));

    glBindVertexArray(0);
}

#pragma once

#include <glad/glad.h>
#include <array>
#include <glm.hpp>
#include <string.h>
#include <vector>

#include "gfx/index_buffer.h"
#include "gfx/vertex_buffer.h"
#include "gfx/vertex_array.h"
#include "gfx/shader.h"

class CSpriteBatch;

constexpr size_t maxQuadCount = 1000;
constexpr size_t maxVertexCount = maxQuadCount * 4;
constexpr size_t maxIndexCount = maxQuadCount * 6;
constexpr size_t maxTextures = 32;

struct Stats {
    uint32_t drawCount = 0;
    uint32_t quadCount = 0;
};

struct Vertex {
    glm::vec2 Position;
    glm::vec3 Color;
    glm::vec2 TexCoords;
    float texIndex;
};

struct RenderData {
    

    CVertexArray* quadVA = nullptr;
    CVertexBuffer* quadVB = nullptr;
    CIndexBuffer* quadIB = nullptr;

    GLuint whiteTexture = 0;
    uint32_t whiteTextureSlot = 0;

    uint32_t indexCount = 0;

    std::vector<Vertex> quadBuffer;

    std::array<uint32_t, maxTextures> textureSlots;
    uint32_t textureSlotIndex = 1;

    Stats renderStats;

};

class CSpriteBatch {
public:
    CSpriteBatch(CShader& shader) : m_shader(shader) { init(); }
    ~CSpriteBatch() { dispose(); }

    void getProjection(const glm::mat4& projection);
    void begin();
    void end();
    void flush();

    void drawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec3& color);
    void drawQuad(const glm::vec2& pos, const glm::vec2& size, uint32_t textureID);
    //void drawQuad(CSprite& sprite);

    const Stats& getStats();
    void resetStats();

    inline int getQuadBufferSize() const { return m_data.quadBuffer.size(); }
    
private:
    void init();
    void dispose();

    RenderData m_data;
    CShader m_shader;

    

    
};

#include "sprite_batch.h"

#include <gtc/type_ptr.hpp>
#include <SDL.h>

void CSpriteBatch::init() {

   if (m_data.quadBuffer.capacity() == 0)   
        m_data.quadBuffer.reserve(maxVertexCount);

    if (m_data.quadVA != nullptr)
        SDL_Log("Spritebatch: Vertex Array is not empty\n");
    m_data.quadVA = new CVertexArray();

    if (m_data.quadVB != nullptr)
        SDL_Log("Spritebatch: Vertex Buffer is not empty\n");
    m_data.quadVB = new CVertexBuffer(nullptr, maxVertexCount * sizeof(Vertex), GL_DYNAMIC_DRAW);
    
    CVertexBufferLayout layout;
    layout.push(GL_FLOAT, 2);
    layout.push(GL_FLOAT, 3);
    layout.push(GL_FLOAT, 2);
    layout.push(GL_FLOAT, 1);
    m_data.quadVA->addBuffer(*m_data.quadVB, layout);

    uint32_t indicies[maxIndexCount];
    uint32_t offset = 0;
    for(int i = 0; i < maxIndexCount; i += 6) {
        indicies[i + 0] = 0 + offset;
        indicies[i + 1] = 1 + offset;
        indicies[i + 2] = 2 + offset;

        indicies[i + 3] = 2 + offset;
        indicies[i + 4] = 3 + offset;
        indicies[i + 5] = 0 + offset;

        offset += 4;
    }

    if (m_data.quadIB != nullptr)
        SDL_Log("Spritebatch: Index Buffer is not empty\n");
    m_data.quadIB = new CIndexBuffer(indicies, maxIndexCount);

    glGenTextures(1, &m_data.whiteTexture);
    glBindTexture(GL_TEXTURE_2D, m_data.whiteTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    uint32_t color = 0xffffffff;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_data.textureSlots[0] = m_data.whiteTexture;
    for (size_t i = 1; i < maxTextures; i++) 
        m_data.textureSlots[i] = 0;
    
    m_data.quadVB->unbind();
    m_data.quadIB->unbind();
}

void CSpriteBatch::dispose() {
    delete m_data.quadVA;
    delete m_data.quadVB;
    delete m_data.quadIB;

    glDeleteTextures(1, &m_data.whiteTexture);
}

void CSpriteBatch::getProjection(const glm::mat4& projection) {
    m_shader.bind();
    glUniformMatrix4fv(glGetUniformLocation(m_shader.getID(), "projection"),1, false, glm::value_ptr(projection));
}

void CSpriteBatch::begin() {
    resetStats();
    m_data.quadBuffer.clear();
}

void CSpriteBatch::end() {
    size_t size = (m_data.quadBuffer.size() * sizeof(Vertex));
    m_data.quadVB->bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, m_data.quadBuffer.data());
    m_data.quadVB->unbind();
}

void CSpriteBatch::flush() {
    for (uint32_t i = 0; i < m_data.textureSlotIndex; i++)
        glBindTextureUnit(i, m_data.textureSlots[i]);

    m_data.quadVA->bind();
    glDrawElements(GL_TRIANGLES, m_data.indexCount, GL_UNSIGNED_INT, m_data.quadIB->getData());
    m_data.renderStats.drawCount++;
    m_data.quadVA->unbind();
    m_shader.unbind();

    m_data.indexCount = 0;
    m_data.textureSlotIndex = 1;
    
}

void CSpriteBatch::drawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec3& color) {
   
    if (m_data.indexCount >= maxIndexCount || m_data.quadBuffer.size() >= maxVertexCount) {
        end();
        flush();
        begin();
    }

    float textureIndex = 0.0f;
    
    if (m_data.quadBuffer.capacity() == 0)
        SDL_Log("Vector jest pusty\n");

    Vertex quad;
    quad.Position = {pos.x, pos.y};
    quad.Color = color;
    quad.TexCoords = { 0.0f, 0.0f};
    quad.texIndex = textureIndex;
    m_data.quadBuffer.push_back(quad);

    quad.Position = {pos.x + size.x, pos.y};
    quad.Color = color;
    quad.TexCoords = { 1.0f, 0.0f};
    quad.texIndex = textureIndex;
    m_data.quadBuffer.push_back(quad);

    quad.Position = {pos.x + size.x, pos.y + size.y};
    quad.Color = color;
    quad.TexCoords = { 1.0f, 1.0f};
    quad.texIndex = textureIndex;
    m_data.quadBuffer.push_back(quad);

    quad.Position = {pos.x, pos.y + size.y};
    quad.Color = color;
    quad.TexCoords = { 0.0f, 1.0f};
    quad.texIndex = textureIndex;
    m_data.quadBuffer.push_back(quad);

    m_data.indexCount += 6;
    m_data.renderStats.quadCount++;
}

void CSpriteBatch::drawQuad(const glm::vec2& pos, const glm::vec2& size, uint32_t textureID) {
    if (m_data.indexCount >= maxIndexCount || m_data.textureSlotIndex > 31) {
        end();
        flush();
        begin();
    }

    const glm::vec3 color = glm::vec3(1.0f);

    float textureIndex = 0.0f;

    for (uint32_t i = 1; i < m_data.textureSlotIndex; i++) {
        if (m_data.textureSlots[i] == textureID) {
            textureIndex = (float)i;
            break;
        }
    }

    if (textureIndex == 0.0f) {
        textureIndex = (float)m_data.textureSlotIndex;
        m_data.textureSlots[m_data.textureSlotIndex] = textureID;
        m_data.textureSlotIndex++;
    }

    if (m_data.quadBuffer.capacity() == 0)
        SDL_Log("Vector jest pusty\n");

    Vertex quad[4];
    quad[0].Position = {pos.x, pos.y};
    quad[0].Color = color;
    quad[0].TexCoords = { 0.0f, 0.0f};
    quad[0].texIndex = textureID;
    m_data.quadBuffer.push_back(quad[0]);

    quad[1].Position = {pos.x + size.x, pos.y};
    quad[1].Color = color;
    quad[1].TexCoords = { 1.0f, 0.0f};
    quad[1].texIndex = textureID;
    m_data.quadBuffer.push_back(quad[1]);

    quad[2].Position = {pos.x + size.x, pos.y + size.y};
    quad[2].Color = color;
    quad[2].TexCoords = { 1.0f, 1.0f};
    quad[2].texIndex = textureID;
    m_data.quadBuffer.push_back(quad[2]);

    quad[3].Position = {pos.x, pos.y + size.y};
    quad[3].Color = color;
    quad[3].TexCoords = { 0.0f, 1.0f};
    quad[3].texIndex = textureID;
    m_data.quadBuffer.push_back(quad[3]);

    m_data.indexCount += 6;
    m_data.renderStats.quadCount++;
}

const Stats& CSpriteBatch::getStats() {
    return m_data.renderStats;
}

void CSpriteBatch::resetStats() {
    memset(&m_data.renderStats, 0, sizeof(Stats));
}
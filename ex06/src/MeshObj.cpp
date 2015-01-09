#include "MeshObj.h"
#include <iostream>
#include <limits>

MeshObj::MeshObj() {
  mVAO = 0;
  mVBO_position = 0;
  mVBO_normal = 0;
  // HINT : texture coords are not used yet
  mIBO = 0;
  mIndexCount = 0;
}

MeshObj::~MeshObj() {
  glDeleteBuffers(1, &mIBO);
  glDeleteBuffers(1, &mVBO_position);
  glDeleteBuffers(1, &mVBO_normal);
  // HINT : texture coords are not used yet

  glDeleteVertexArrays(1, &mVAO);
}

void MeshObj::setData(const MeshData &meshData) {
  mIndexCount = meshData.indices.size();
  
  // TODO: create local storage arrays for vertices, normals and indices //
  unsigned int vertexDataSize = meshData.vertex_position.size();
  unsigned int vertexNormalSize = meshData.vertex_normal.size();

  GLfloat *vertex_position = new GLfloat[vertexDataSize]();
  GLfloat *vertex_normal = new GLfloat[vertexDataSize]();
  // HINT : texture coords are not used yet

  GLuint *indices = new GLuint[mIndexCount]();
  
  // TODO: copy data into local arrays //
  std::copy(meshData.vertex_position.begin(), meshData.vertex_position.end(), vertex_position);
  std::copy(meshData.vertex_normal.begin(), meshData.vertex_normal.end(), vertex_normal);
  std::copy(meshData.indices.begin(), meshData.indices.end(), indices);
  
  // TODO: create VAO //
  if (mVAO == 0) {
    glGenVertexArrays(1, &mVAO);
  }
  glBindVertexArray(mVAO);
  
  // TODO: create and bind VBOs and upload data (one VBO per vertex attribute -> position, normal) //
  if (mVBO_position == 0) {
    glGenBuffers(1, &mVBO_position);
  }
  glBindBuffer(GL_ARRAY_BUFFER, mVBO_position);
  glBufferData(GL_ARRAY_BUFFER, vertexDataSize * sizeof(GLfloat), &vertex_position[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);
    
  if (mVBO_normal == 0) {
    glGenBuffers(1, &mVBO_normal);
  }
  glBindBuffer(GL_ARRAY_BUFFER, mVBO_normal);
  glBufferData(GL_ARRAY_BUFFER, vertexDataSize * sizeof(GLfloat), &vertex_normal[0], GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(1);

  // HINT : texture coords are not used yet
  
  // TODO: init and bind a IBO //
  if (mIBO == 0) {
    glGenBuffers(1, &mIBO);
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexCount * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
  
  // unbind buffers //
  glBindVertexArray(0);
  
  // TODO: make sure to clean up temporarily allocated data, if neccessary //
  delete[] vertex_position;
  if (vertexNormalSize > 0) {
    delete[] vertex_normal;
  }
  delete[] indices;
}

void MeshObj::render(void) {
  // TODO: render your VAO //
  if (mVAO != 0) {
    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
  }
}

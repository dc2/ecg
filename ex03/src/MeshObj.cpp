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
    
    //std::cout << mIndexCount << std::endl;
    
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    
    glGenBuffers(1, &mVBO_position);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO_position);
    
    // TODO: copy data into the VBO //
    glBufferData(GL_ARRAY_BUFFER, meshData.vertex_position.size()*sizeof(GLfloat), meshData.vertex_position.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    
    glGenBuffers(1, &mVBO_normal);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO_normal);
    
    glBufferData(GL_ARRAY_BUFFER, meshData.vertex_normal.size()*sizeof(GLfloat), meshData.vertex_normal.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    
    // TODO: init and bind a IBO (index buffer object) //
    glGenBuffers(1, &mIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
    
    // TODO: copy data into the IBO //
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexCount*sizeof(GLint), meshData.indices.data(), GL_STATIC_DRAW);
    
    mIndexCount =  meshData.indices.size();
    


    // TODO: create local storage arrays for vertices, normals and indices //
    
    // TODO: copy data into local arrays //
    
    // TODO: create VAO //
    
    // TODO: create and bind VBOs and upload data (one VBO per vertex attribute -> position, normal) //
    
    // TODO: init and bind a IBO //

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void MeshObj::render(void) {
    if (mVAO != 0) {
        // TODO: render your VAO //
        
        glBindVertexArray(mVAO);
        
        glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0);
        
        glBindVertexArray(0);
    }
}

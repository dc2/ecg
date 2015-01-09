#include "MeshObj.h"
#include <iostream>
#include <limits>
#include <SDL2/SDL_image.h>

MeshObj::MeshObj() {
  mVAO = 0;
  mVBO_position = 0;
  mVBO_normal = 0;
  mVBO_texcoord = 0;
  mIBO = 0;
  mIndexCount = 0;
}

MeshObj::~MeshObj() {
  glDeleteBuffers(1, &mIBO);
  glDeleteBuffers(1, &mVBO_position);
  glDeleteBuffers(1, &mVBO_normal);
  glDeleteBuffers(1, &mVBO_texcoord);
  glDeleteVertexArrays(1, &mVAO);
}

void MeshObj::setData(const MeshData &meshData) {
  mIndexCount = meshData.indices.size();
  
  // TODO: extend this method to upload texture coordinates as another VBO //
  // - texture coordinates are at location 2 within the shader code
  
  // create local storage arrays for vertices, normals and indices //
  unsigned int vertexDataSize = meshData.vertex_position.size();
  unsigned int vertexNormalSize = meshData.vertex_normal.size();
  unsigned int vertexTexcoordSize = meshData.vertex_texcoord.size();
  
  // create VAO //
  if (mVAO == 0) {
    glGenVertexArrays(1, &mVAO);
  }
  glBindVertexArray(mVAO);
  
  // create and bind VBOs and upload data (one VBO per available vertex attribute -> position, normal) //
  if (mVBO_position == 0) {
    glGenBuffers(1, &mVBO_position);
  }
  glBindBuffer(GL_ARRAY_BUFFER, mVBO_position);
  glBufferData(GL_ARRAY_BUFFER, vertexDataSize * sizeof(GLfloat), meshData.vertex_position.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);
  
  if (vertexNormalSize > 0) {
    if (mVBO_normal == 0) {
      glGenBuffers(1, &mVBO_normal);
    }
    glBindBuffer(GL_ARRAY_BUFFER, mVBO_normal);
    glBufferData(GL_ARRAY_BUFFER, vertexNormalSize * sizeof(GLfloat), meshData.vertex_normal.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
  }
  
  if (vertexTexcoordSize > 0) {
    if (mVBO_texcoord == 0) {
      glGenBuffers(1, &mVBO_texcoord);
    }
    glBindBuffer(GL_ARRAY_BUFFER, mVBO_texcoord);
    glBufferData(GL_ARRAY_BUFFER, vertexTexcoordSize * sizeof(GLfloat), meshData.vertex_texcoord.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(2);
  }
  
  // init and bind a IBO //
  if (mIBO == 0) {
    glGenBuffers(1, &mIBO);
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexCount * sizeof(GLuint), meshData.indices.data(), GL_STATIC_DRAW);
  
  // unbind buffers //
  glBindVertexArray(0);
}

void MeshObj::render(void) {
  // render your VAO //
  if (mVAO != 0) {
    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
  }
}


void TextureData::init(std::string file) {
    SDL_Surface *image = IMG_Load(file.c_str());
    
    if(image)
    {
        width = image->w;
        height = image->h;
        
        glGenTextures(1, &texture);
        
        if(!texture)
        {
            return;
            //throw std::runtime_error("Create Texture failed");
        }
        
        GLint internalFormat = GL_RGB;
        GLenum format = GL_RGB;
        
        switch(image->format->BytesPerPixel)
        {
        case 1:
            internalFormat = GL_R8; format = GL_RED;
            break;
        case 2:
            internalFormat = GL_R16; format = GL_RED;
            break;
        case 3:
            internalFormat = GL_RGB8; format = GL_RGB;
            break;
            
        default:
        case 4:
            internalFormat = GL_RGBA; format = GL_RGBA;
            break;
        }
        
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, image->pixels);
        //glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
        //glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        
        // Activate MipMapping with trilinear and anisotropic filtering
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4);
        
        SDL_FreeSurface(image);
    }
}


TextureData::TextureData()
{
    
}

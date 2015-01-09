#ifndef __MESH_OBJ__
#define __MESH_OBJ__

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <vector>
#include <stack>
#include <string>

struct TextureData {
    void init(std::string file);
    
    unsigned int width;
    unsigned int height;
    
    GLuint texture;
};

struct MeshData {
    // data vectors //
    std::vector<GLfloat> vertex_position;
    std::vector<GLfloat> vertex_normal;
    std::vector<GLfloat> vertex_texcoord;
    // index list //
    std::vector<GLuint> indices;
};

class MeshObj {
public:
    MeshObj();
    ~MeshObj();
    
    void setData(const MeshData &data, GLint texture = -1);
    void render(void);
    
private:
    GLint mTexture;
    
    GLuint mVAO;
    GLuint mVBO_position;
    GLuint mVBO_normal;
    GLuint mVBO_texcoord;
    GLuint mIBO;
    GLuint mIndexCount;
};

class Mesh {
public:
    ~Mesh();
    
    void addMesh(MeshObj *mesh);
    void render();
private:
    std::vector<MeshObj*> m_meshes;
};

#endif

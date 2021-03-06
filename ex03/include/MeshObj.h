#ifndef __MESH_OBJ__
#define __MESH_OBJ__

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <vector>
#include <stack>

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
    
    void setData(const MeshData &data);
    void render(void);
    
  private:
    GLuint mVAO;
    GLuint mVBO_position;
    GLuint mVBO_normal;
    // HINT : texture coords are not used yet
    GLuint mIBO;
    GLuint mIndexCount;
};

#endif

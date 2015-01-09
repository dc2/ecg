#ifndef __MESH_OBJ__
#define __MESH_OBJ__

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <vector>
#include <stack>
#include <string>

// NEW: texture stuff //
// - Container for texture data
struct TextureData {
  TextureData();
  void init(std::string file);
  unsigned char *data;
  unsigned int width;
  unsigned int height;
  // - OpenGL texture handle
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
    
    void setData(const MeshData &data);
    void render(void);

	// - method to initialize the texture object
	void initTextures();
	// - method to load a texture from a given file
	TextureData loadTextureData(const char *fileName);
    
  private:
    GLuint mVAO;
    GLuint mVBO_position;
    GLuint mVBO_normal;
    GLuint mVBO_texcoord;
    GLuint mIBO;
    GLuint mIndexCount;
};

#endif

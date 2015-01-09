#ifndef __OBJ_LOADER__
#define __OBJ_LOADER__

#include <map>
#include <string>

#include <glm/glm.hpp>

#include "MeshObj.h"

class ObjLoader {
  public:
    ObjLoader();
    ~ObjLoader();
    Mesh* loadObjFile(const std::string &fileName, const std::string &ID = "");
    Mesh *getMeshObj(const std::string &ID);
    
    std::string getMaterialTexture(const std::string &fileName, const std::string &usemtl);
    
  private:
    std::map<std::string, Mesh*> mMeshMap;
};

#endif

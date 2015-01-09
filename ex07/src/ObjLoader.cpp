#include "ObjLoader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

ObjLoader::ObjLoader() {
}

ObjLoader::~ObjLoader() {
  for (std::map<std::string, MeshObj*>::iterator iter = mMeshMap.begin(); iter != mMeshMap.end(); ++iter) {
    delete iter->second;
    iter->second = NULL;
  }
  mMeshMap.clear();
}

MeshObj* ObjLoader::loadObjFile(std::string fileName, std::string ID) {
  // sanity check for identfier -> must not be empty //
  if (ID.length() == 0) {
    return NULL;
  }
  // try to load the MeshObj for ID //
  MeshObj* meshObj = getMeshObj(ID);
  if (meshObj != NULL) {
    // if found, return it instead of loading a new one from file //
    return meshObj;
  }
  // ID is not known yet -> try to load mesh from file //
  
  // import mesh from given file //
  // setup variables used for parsing //
  std::string key;
  float x, y, z;

  // local lists //
  // these lists are used to store the imported information
  // before putting the data into the MeshData container for the MeshObj
  std::vector<glm::vec3> localVertexPosition;
  std::vector<glm::vec3> localVertexNormal;
  std::vector<glm::vec2> localVertexTexcoord;
  std::vector<std::vector<glm::vec3>> localFace;
  
  // setup tools for parsing a line correctly //
  std::string line;
  std::stringstream sstr;

  // open file //
  unsigned int lineNumber = 0;
  std::ifstream file(fileName.c_str());
  if (file.is_open()) {
    while (file.good()) {
      key = "";
      getline(file, line);
      sstr.clear();
      sstr.str(line);
      sstr >> key;
      if (!key.compare("v")) {
        // read in vertex //
        sstr >> x >> y >> z;
		localVertexPosition.push_back(glm::vec3(x, y, z));
      }
      // implement parsing of vertex normal and tecture coordinate //
      if (!key.compare("vn")) {
        // read in vertex normal //
        sstr >> x >> y >> z;
		localVertexNormal.push_back(glm::vec3(x, y, z));
      }
      if (!key.compare("vt")) {
        // read in texture coordinate //
        sstr >> x >> y;
		localVertexTexcoord.push_back(glm::vec2(x, y));
      }

      if (!key.compare("f")) {

		// read in vertex indices for a face //
        unsigned int vCount = 0;

		// indices for vertex (vi), normal (ni) and texture (ti) //
		int vi[4];
		int ni[4];
		int ti[4];
  
        sstr.peek();
        while (sstr.good() && vCount < 4) {
          sstr >> vi[vCount];
		  //std::cout << vi[vCount] << std::endl;
          if (!sstr.good()) {
            // import of vertex index failed -> end of line reached //
            vi[vCount] = 0;
          } else {
            // vertex index import successful -> try to load texture and normal indices //
            if (sstr.peek() == '/') {
              sstr.get(); // skip '/' symbol //
              if (sstr.peek() != '/') {
                // there is a texture coordinate //
                sstr >> ti[vCount];
              } else {
                ti[vCount] = 0;
              }
              sstr.get(); // skip '/' symbol //
              sstr >> ni[vCount];
            } else {
              ti[vCount] = 0;
              ni[vCount] = 0;
            }
            ++vCount;
          }
        }
        
        // insert index data into face //
        if (vCount < 3) {
          std::cout << "(ObjLoader::loadObjFile) - WARNING: Malformed face in line " << lineNumber << std::endl;
          continue; // not a real face //
        } else if (vCount > 3) {
          // quad face loaded -> split into two triangles (0,1,2) and (0,2,3) //
          std::vector<glm::vec3> face0, face1;
          for (unsigned int v = 0; v < vCount; ++v) {
            if (v != 3) {
				face0.push_back(glm::vec3(vi[v] - 1, ni[v] - 1, ti[v] - 1));
            }
            if (v != 1) {
              face1.push_back(glm::vec3(vi[v] - 1, ni[v] - 1, ti[v] - 1));
            }
          }
          localFace.push_back(face0);
          localFace.push_back(face1);
        } else {
          std::vector<glm::vec3> face;
          for (unsigned int v = 0; v < vCount; ++v) {
			face.push_back(glm::vec3(vi[v] - 1, ni[v] - 1, ti[v] - 1));
          }
          localFace.push_back(face);
        }
      }
      ++lineNumber;
    }
    file.close();
    
    std::cout << fileName << " has been imported." << std::endl;
	std::cout << "  Vertex count : " << localVertexPosition.size() << std::endl;
	std::cout << "  Normal count = " << localVertexNormal.size() << std::endl;
	std::cout << "  Tex coord count = " << localVertexTexcoord.size() << std::endl;
	std::cout << "  Face count = " << localFace.size() << std::endl;
    
    MeshData meshData;

	// setup variables used for parsing //
    std::map<std::string, unsigned int> vertexIdMap;
	bool writeTexCoords = false;
	if (localVertexTexcoord.size() != 0) {
		writeTexCoords = true;
	}

    for (std::vector<std::vector<glm::vec3> >::iterator faceIter = localFace.begin();
		faceIter != localFace.end(); ++faceIter)
	{

      std::string vertexId("");
      const char* idPattern = "%08d|%08d|%08d";
      char idStr[27];

      // iterate over face vertices //
      for (unsigned int i = 0; i < 3; ++i) {

		int vi = (*faceIter)[i][0];
		int ni = (*faceIter)[i][1];
		int ti = (*faceIter)[i][2];
        sprintf(idStr, idPattern, vi, ni, ti);
        vertexId = std::string(idStr);

		// vertexId is now "vertexIndex/normalIndex/textureIndex" //
        std::map<std::string, unsigned int>::iterator vertexIdIter = vertexIdMap.find(vertexId);
        if (vertexIdIter == vertexIdMap.end()) {

          // vertex not known yet -> insert new one //
          unsigned int newIndex = meshData.vertex_position.size() / 3;

		  // add vertex position data //
          glm::vec3 position = localVertexPosition[vi];
          meshData.vertex_position.push_back(position.x);
		  meshData.vertex_position.push_back(position.y);
		  meshData.vertex_position.push_back(position.z);

          // add vertex normal data //
          glm::vec3 normal = localVertexNormal[ni];
          meshData.vertex_normal.push_back(normal.x);
		  meshData.vertex_normal.push_back(normal.y);
		  meshData.vertex_normal.push_back(normal.z);

		  if (writeTexCoords) {
			  // add vertex texture coord data //
			  glm::vec2 texcoord = localVertexTexcoord[ti];
			  meshData.vertex_texcoord.push_back(texcoord.x);
			  meshData.vertex_texcoord.push_back(texcoord.y);
			  // insert new index into index list AND index map //
		  }
		  meshData.indices.push_back((GLuint)newIndex);
		  vertexIdMap.insert(std::make_pair(vertexId, newIndex));
        } else {
		  // vertex already defined -> use index and add to indexList //
          meshData.indices.push_back((GLuint)vertexIdIter->second);
        }
      }
    }
    
    // create new MeshObj and set imported geoemtry data //
    meshObj = new MeshObj();
    // assign imported data to this new MeshObj //
    meshObj->setData(meshData);
    
    // insert MeshObj into map //
    mMeshMap.insert(std::make_pair(ID, meshObj));
    
    // return newly created MeshObj //
    return meshObj;
  } else {
    std::cout << "(ObjLoader::loadObjFile) : Could not open file: \"" << fileName << "\"" << std::endl;
    return NULL;
  }
}

MeshObj* ObjLoader::getMeshObj(std::string ID) {
  // sanity check for ID //
  if (ID.length() > 0) {
    std::map<std::string, MeshObj*>::iterator mapLocation = mMeshMap.find(ID);
    if (mapLocation != mMeshMap.end()) {
      // mesh with given ID already exists in meshMap -> return this mesh //
      return mapLocation->second;
    }
  }
  // no MeshObj found for ID -> return NULL //
  return NULL;
}

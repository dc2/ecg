#ifndef __EX03__
#define __EX03__

// Include standard headers
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stack>

// Include GLEW
#include <GL/glew.h>

// Include GLUT
#include <GL/freeglut.h>

// Include GLM
#define GLM_SWIZZLE 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp> 
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

// TODO : Include AntTweakBar
#include <AntTweakBar.h>

// local includes
#include "ObjLoader.h"
#include "CameraController.h"

std::stack<glm::mat4> glm_ProjectionMatrix; 
std::stack<glm::mat4> glm_ModelViewMatrix; 

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

#endif

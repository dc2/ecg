#ifndef __EX03__
#define __EX03__

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp> 

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stack>

#include "ObjLoader.h"

std::stack<glm::mat4> glm_ProjectionMatrix; 
std::stack<glm::mat4> glm_ModelViewMatrix; 

#endif

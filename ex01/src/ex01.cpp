#include "ex01.h"
void triangleSolution(void)
{
    glm::vec3 a(4, 1, 0), b(-1, 3, 2), c(0, 2, 1);  
    glm::vec3 v(1, 1, -3); v = glm::normalize(v);
    
    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;
    
    glm::vec3 normal = glm::normalize(glm::cross(ac, ab));
    
    
    
    std::cout << glm::to_string(normal) << std::endl << glm::angle(normal, v) << "°" << std::endl;
}

int main (int argc, char **argv) {
  triangleSolution();
  
  return 0;
}

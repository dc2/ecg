#include "Ex03.h"

#include <glm/gtx/transform.hpp>

// OpenGL and GLSL stuff //
void initGL();
void initShader();
bool enableShader();
void disableShader();
void deleteShader();
char* loadShaderSource(const char* fileName);
bool glError(char *msg);

GLuint loadShaderFile(const char* fileName, GLenum shaderType);
GLuint shaderProgram = 0;
GLint uniform_projectionMatrix;
GLint uniform_modelViewMatrix;

// window controls //
void updateGL();
void idle();
void keyboardEvent(unsigned char key, int x, int y);

// geometry //
void initScene();

int main (int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitContextVersion(3,3);
  glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  glutInitWindowSize (512, 512);
  glutInitWindowPosition (100, 100);
  glutCreateWindow("Exercise 03 - More creatures ... feeding time!");
  
  glutDisplayFunc(updateGL);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyboardEvent);
  
  glError("creating rendering context");
  glewExperimental = true;
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cout << "(glewInit) - Error: " << glewGetErrorString(err) << std::endl;
  }
  std::cout << "(glewInit) - Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  glError("rendering context created");
  
  // init stuff //
  initGL();
  glError("GL initialized");
  
  // init matrix stacks //
  glm_ProjectionMatrix.push(glm::mat4(2.414214, 0.000000, 0.000000, 0.000000, 0.000000, 2.414214, 0.000000, 0.000000, 0.000000, 0.000000, -1.002002, -1.000000, 0.000000, 0.000000, -0.020020, 0.000000));
  glm_ModelViewMatrix.push(glm::mat4(0.707107, -0.408248, 0.577350, 0.000000, 0.000000, 0.816497, 0.577350, 0.000000, -0.707107, -0.408248, 0.577350, 0.000000, 0.000000, 0.000000, -1.732051, 1.000000));
  
  initShader();
  glError("shader initialized");

  initScene();
  glError("scene initialized");
  
  // start render loop //
  if (enableShader()) {
    glError("enableShader()");

    glutMainLoop();

    disableShader();
  }
  
  return 0;
}

bool glError(char *msg) {
    // check OpenGL error
    GLenum err;
    bool err_occured = false;
    while ((err = glGetError()) != GL_NO_ERROR) {
        err_occured = true;
        std::cerr << "OpenGL error ( " << msg << " ) : " << err << std::endl;
    }
    return err_occured;
}

void initGL() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_DEPTH_TEST);
}

void initShader() {

  glError("begin initShader()");

  shaderProgram = glCreateProgram();
  // check if operation failed //
  if (shaderProgram == 0) {
    std::cout << "(initShader) - Failed creating shader program." << std::endl;
    return;
  }
  glError("glCreateProgram()");
  
  GLuint vertexShader = loadShaderFile("shader/simple.vert", GL_VERTEX_SHADER);
  if (vertexShader == 0) {
    std::cout << "(initShader) - Could not create vertex shader." << std::endl;
    deleteShader();
    return;
  }
  glError("loadShaderFile(GL_VERTEX_SHADER)");

  GLuint fragmentShader = loadShaderFile("shader/simple.frag", GL_FRAGMENT_SHADER);
  if (fragmentShader == 0) {
    std::cout << "(initShader) - Could not create vertex shader." << std::endl;
    deleteShader();
    return;
  }
  glError("loadShaderFile(GL_FRAGMENT_SHADER)");
  
  // successfully loaded and compiled shaders -> attach them to program //
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glError("glAttachShader()");
  
  // mark shaders for deletion after clean up (they will be deleted, when detached from all shader programs) //
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  glError("glDeleteShader()");
  
  // link shader program //
  glLinkProgram(shaderProgram);
  glError("glLinkProgram()");

  
  // get log //
  GLint status;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
  glError("glGetProgramiv()");

  if (status == GL_FALSE)
  {
    GLint infoLogLength;
    glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

    GLchar* strInfoLog = new GLchar[infoLogLength + 1];
    glGetShaderInfoLog(shaderProgram, infoLogLength, NULL, strInfoLog);
    glError("glGetShaderInfoLog");

    std::cout << "(initShader) - Linker log:\n------------------\n" << strInfoLog << "\n------------------" << std::endl;
    delete[] strInfoLog;
    glError("strInfoLog()");
  }
  
  // set address of fragment color output //
  glBindFragDataLocation(shaderProgram, 0, "color");
  glError("glBindFragDataLocation()");
}

bool enableShader() {
  if (shaderProgram > 0) {
    glUseProgram(shaderProgram);
  } else {
    std::cout << "(enableShader) - Shader program not initialized." << std::endl;
  }
  return shaderProgram > 0;
}

void disableShader() {
  glUseProgram(0);
}

void deleteShader() {
  // use standard pipeline //
  glUseProgram(0);
  // delete shader program //
  glDeleteProgram(shaderProgram);
  shaderProgram = 0;
  glError("deleteShader()");
}

// load and compile shader code //
char* loadShaderSource(const char* fileName) {
  char *shaderSource = NULL;
  
  std::ifstream file(fileName, std::ios::in);
  if (file.is_open()) {
    unsigned long srcLength = 0;
    file.seekg(0, std::ios::end);
    srcLength = file.tellg();
    file.seekg(0, std::ios::beg);
    
    shaderSource = new char[srcLength + 1];
    shaderSource[srcLength] = 0;
    file.read(shaderSource, srcLength);
    file.close();
  } else {
    std::cout << "(loadShaderSource) - Could not open file \"" << fileName << "\"." << std::endl;
  }
  
  return shaderSource;
}

// loads a source file and directly compiles it to a shader of 'shaderType' //
GLuint loadShaderFile(const char* fileName, GLenum shaderType) {
  GLuint shader = glCreateShader(shaderType);
  // check if operation failed //
  if (shader == 0) {
    std::cout << "(loadShaderFile) - Could not create shader." << std::endl;
    return 0;
  }
  
  // load source code from file //
  const char* shaderSrc = loadShaderSource(fileName);
  if (shaderSrc == NULL) return 0;
  // pass source code to new shader object //
  glShaderSource(shader, 1, (const char**)&shaderSrc, NULL);
  delete[] shaderSrc;
  // compile shader //
  glCompileShader(shader);
  
  // log compile messages, if any //
  GLint logMaxLength;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logMaxLength);
  GLchar *log = new GLchar[logMaxLength+1];
  GLint logLength = 0;
  glGetShaderInfoLog(shader, logMaxLength, &logLength, log);
  if (logLength > 0) {
    std::cout << "(loadShaderFile) - Compiler log:\n------------------\n" << log << "\n------------------" << std::endl;
  }
  delete[] log;
  
  // return compiled shader (may have compiled WITH errors) //
  return shader;
}

ObjLoader objLoader;

void initScene() {
  // load bunny.obj and armadillo.obj from disk and create renderable MeshObj //
  objLoader.loadObjFile("meshes/armadillo.obj", "armadillo");
  objLoader.loadObjFile("meshes/bunny.obj", "bunny");
}

GLfloat rotAngle = 0;
void updateGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // projection matrix stays the same //
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, false, glm::value_ptr(glm_ProjectionMatrix.top()));
  
  // init scene graph by cloning the top entry, which can now be manipulated //
  glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());
  
  // TODO: create a rotating grid of Stanford models (2 x 2 grid)
  //  - render one pair of bunny and armadillo in every quadrant of the grid (-> obj files loaded in initScene() already)
  //
  //    sketch: notation: B = bunny, A = armadillo
  //
  //          B       B
  //            A | A   
  //          ----------         
  //            A | A  
  //          B   |   B
  //
  //  - rotate the scene clockwise about 'rotAngle'
  //  - rotate the armadillos and bunnies so that each pair "looks" at each other (compare to image in pdf)
  //  - use glm_ModelViewMatrix.push(...) and glm_ModelViewMatrix.pop()   
  //  - apply new transformations by using: glm_ModelViewMatrix.top() *= glm::some_transformation(...);
  //  - right before rendering an object, upload the current state of the modelView matrix stack:
  //    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
  
  
  for(int x=-1; x <= 1; x+=2)
  {
      for(int y=-1; y <= 1; y+=2)
      {
          glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());
          
          glm_ModelViewMatrix.top() *= glm::rotate(rotAngle, glm::vec3(0, 1, 0));
          glm_ModelViewMatrix.top() *= glm::translate(glm::vec3(0.5*x, 0, 0.5*y));
          glm_ModelViewMatrix.top() *= glm::scale(glm::vec3(0.1, 0.1, 0.1)); 
          glm_ModelViewMatrix.top() *= glm::rotate(42.f, glm::vec3(0, 1, 0));
          
          glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
          objLoader.getMeshObj("bunny")->render();
          
          glm_ModelViewMatrix.pop();
      }
  }

  
  
  // restore scene graph to previous state //
  glm_ModelViewMatrix.pop();
  
  // increment rotation angle //
  rotAngle += 0.5f;
  if (rotAngle > 360.0f) rotAngle -= 360.0f;
  
  // swap renderbuffers for smooth rendering //
  glutSwapBuffers();
}

void idle() {
  glutPostRedisplay();
}

void keyboardEvent(unsigned char key, int x, int y) {
  if (key == 'x' || key == 27) {
    exit(0);
  }
  glutPostRedisplay();
}

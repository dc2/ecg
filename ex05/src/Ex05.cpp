#include "Ex05.h"


#include <glm/gtx/transform.hpp>

// OpenGL and GLSL stuff //
void initGL();
void initShader();
bool enableShader();
void disableShader();
void deleteShader();
bool glError(char *msg);
void printShaderInfoLog(GLuint shader);
void printProgramInfoLog(GLuint program);
GLuint loadShaderFile(const char* fileName, GLenum shaderType);
// the used shader program //
GLuint shaderProgram = 0;
// this map stores uniform locations of our shader program //
std::map<std::string, GLint> uniformLocations;

// window controls //
void updateBar();
void updateGL();
void idle();
void close();
void Reshape(int width, int height);
void keyboardEvent(unsigned char key, int x, int y);
void mouseEvent(int button, int state, int x, int y);
void mouseMoveEvent(int x, int y);

// scene controls //
CameraController camera(0, 0, 1.0);			// camera looks frontal on scene, distance from origin = 1.0
bool rotAnim = true;
GLfloat rotAngle = 0;
// properties used for pumping animation of the model
bool pumpAnim = true;
GLfloat timeValue = 0.0f;
GLfloat pumpSpeed = 0.6f;
GLfloat pumpAmplitude = 0.01f;

// viewport //
TwBar * CameraGUI;
GLint windowWidth, windowHeight;
GLint guiWidth;

// geometry //
void initScene();
void renderScene();

// OBJ import //
ObjLoader *objLoader;

// these structs are also used in the shader code  //
// this helps to access the parameters more easily //
struct Material {
    glm::vec3 ambient_color;
    glm::vec3 diffuse_color;
    glm::vec3 specular_color;
    float specular_shininess;
};

struct LightSource {
    glm::vec3 ambient_color;
    glm::vec3 diffuse_color;
    glm::vec3 specular_color;
    glm::vec3 position;
};

// the program uses a list of materials and light sources, which can be chosen during rendering //
unsigned int materialIndex;
unsigned int materialCount;
std::vector<Material> materials;

Material *currentMaterial = nullptr;
unsigned int lightIndex;
unsigned int lightCount;
std::vector<LightSource> lights;
LightSource *currentLight = nullptr;

int main (int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion(3,3);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    
    windowWidth = 812;
    windowHeight = 512;
    guiWidth = 300;
    
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Exercise 05 - Per Fragment Lighting with GLSL");
    
    glutDisplayFunc(updateGL);
    glutReshapeFunc(Reshape);
    glutIdleFunc(idle);
    
    glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
    glutMouseFunc(mouseEvent);
    glutMotionFunc(mouseMoveEvent);
    glutKeyboardFunc(keyboardEvent);
    glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);
    TwGLUTModifiersFunc(glutGetModifiers);
    
    
    // Initialize the GUI
    TwInit(TW_OPENGL_CORE, NULL);
    TwWindowSize(windowWidth, windowHeight);
    CameraGUI = TwNewBar("CameraGUI");
    TwDefine(" GLOBAL help='Exercise Sheet 04 - Camera controller and projection.' "); // Message added to the help bar.
    
    // set position and size of AntTweakBar
    char *setbarposition = new char[1000];
    sprintf(setbarposition, " CameraGUI position='%i %i' size='%i %i'\0", windowWidth-(guiWidth-10), 20, guiWidth-20, windowHeight-40);
    std::cout << setbarposition << std::endl;
    TwDefine(setbarposition);
    delete[] setbarposition;
    
    glutCloseFunc(close);
    
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cout << "(glewInit) - Error: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "(glewInit) - Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
    
    // init stuff //
    initGL();
    
    // configure Camera
    camera.setFar(20.0);
    camera.setNear(0.01);
    camera.setOpeningAngle(45.0);
    camera.setAspect(1.2);
    
    // init object loader
    objLoader = new ObjLoader();
    
    // init matrix stacks with identity //
    glm_ProjectionMatrix.push(glm::mat4(1));
    glm_ModelViewMatrix.push(glm::mat4(1));
    
    initShader();
    initScene();
    
    // configure GUI editing bars
    updateBar();
    
    // start render loop //
    if (enableShader()) {
        glutMainLoop();
        disableShader();
        
        // clean up allocated data //
        deleteShader();
    }
    
    return 0;
}

void updateBar() {
    
    TwRemoveAllVars(CameraGUI);
    
    TwSetParam(CameraGUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
    
    std::stringstream label;
    
    if (currentMaterial != nullptr) {
        label << " group='Material " << materialIndex << "' ";
        TwAddVarRW(CameraGUI, "Ambient M", TW_TYPE_COLOR3F, &currentMaterial->ambient_color, label.str().c_str());
        TwAddVarRW(CameraGUI, "Diffuse M", TW_TYPE_COLOR3F, &currentMaterial->diffuse_color, label.str().c_str());
        TwAddVarRW(CameraGUI, "Specular M", TW_TYPE_COLOR3F, &currentMaterial->specular_color, label.str().c_str());
        TwAddVarRW(CameraGUI, "Shininess M", TW_TYPE_FLOAT, &currentMaterial->specular_shininess, label.str().c_str());
    }
    
    if (currentLight != nullptr) {
        std::stringstream label2;
        label2 << " group='Light " << lightIndex << "' ";
        TwAddVarRW(CameraGUI, "Ambient L", TW_TYPE_COLOR3F, &currentLight->ambient_color, label2.str().c_str());
        TwAddVarRW(CameraGUI, "Diffuse L", TW_TYPE_COLOR3F, &currentLight->diffuse_color, label2.str().c_str());
        TwAddVarRW(CameraGUI, "Specular L", TW_TYPE_COLOR3F, &currentLight->specular_color, label2.str().c_str());
        
        label << " step = 0.01";
        TwAddVarRW(CameraGUI, "Pos X", TW_TYPE_FLOAT, &currentLight->position.x, label2.str().c_str());
        TwAddVarRW(CameraGUI, "Pos Y", TW_TYPE_FLOAT, &currentLight->position.y, label2.str().c_str());
        TwAddVarRW(CameraGUI, "Pos Z", TW_TYPE_FLOAT, &currentLight->position.z, label2.str().c_str());
        
        label2.clear();
        label2 << " group='Light " << lightIndex << "' ";
    }
    
    TwAddVarRW(CameraGUI, "Rotating Light", TW_TYPE_BOOL8 , &rotAnim, "help='Auto-rotate light?'");
    TwAddVarRW(CameraGUI, "Rotation Angle", TW_TYPE_FLOAT, &rotAngle, "step=1.0");
    TwAddVarRW(CameraGUI, "Pumping Animation", TW_TYPE_BOOL8 , &pumpAnim, "help='Pumping animation?'");
    TwAddVarRW(CameraGUI, "Pumping Speed", TW_TYPE_FLOAT, &pumpSpeed, "step=0.01");
    TwAddVarRW(CameraGUI, "Pumping Amplitude", TW_TYPE_FLOAT, &pumpAmplitude, "step=0.001");
}

// Callback function called by GLUT when window size changes
void Reshape(int width, int height)
{
    // Set OpenGL viewport and camera
    windowWidth = width;
    windowHeight = height;
    
    // Send the new window size to AntTweakBar
    TwWindowSize(windowWidth, windowHeight);
    
    // set position and size of AntTweakBar
    char *setbarposition = new char[1000];
    sprintf(setbarposition, " CameraGUI position='%i %i' size='%i %i'\0", windowWidth-(guiWidth-10), 20, guiWidth-20, windowHeight-40);
    std::cout << setbarposition << std::endl;
    TwDefine(setbarposition);
    delete[] setbarposition;
    
}

void close() {
    
    TwTerminate();
    
    // free allocated objects
    delete objLoader;
    
    std::cout << "Shutdown program." << std::endl;
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

// Print information about the compiling step
void printShaderInfoLog(GLuint shader)
{
    GLint infologLength = 0;
    GLsizei charsWritten  = 0;
    char *infoLog;
    
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH,&infologLength);		
    infoLog = (char *)malloc(infologLength);
    glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
    printf("%s\n",infoLog);
    free(infoLog);
}

// Print information about the linking step
void printProgramInfoLog(GLuint program)
{
    GLint infoLogLength = 0;
    GLsizei charsWritten  = 0;
    char *infoLog;
    
    glGetProgramiv(program, GL_INFO_LOG_LENGTH,&infoLogLength);
    infoLog = (char *)malloc(infoLogLength);
    glGetProgramInfoLog(program, infoLogLength, &charsWritten, infoLog);
    printf("%s\n",infoLog);
    free(infoLog);
}

void initGL() {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
}

void initShader() {
    shaderProgram = glCreateProgram();
    // check if operation failed //
    if (shaderProgram == 0) {
        std::cout << "(initShader) - Failed creating shader program." << std::endl;
        return;
    }
    
    GLuint vertexShader = loadShaderFile("shader/material_and_light.vert", GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        std::cout << "(initShader) - Could not create vertex shader." << std::endl;
        deleteShader();
        return;
    }
    GLuint fragmentShader = loadShaderFile("shader/material_and_light.frag", GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        std::cout << "(initShader) - Could not create vertex shader." << std::endl;
        deleteShader();
        return;
    }
    
    // successfully loaded and compiled shaders -> attach them to program //
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    
    // mark shaders for deletion after clean up (they will be deleted, when detached from all shader programs) //
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // link shader program //
    glLinkProgram(shaderProgram);
    
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
    
    // get uniform locations for common variables //
    uniformLocations["projection"] = glGetUniformLocation(shaderProgram, "projection");
    uniformLocations["view"] = glGetUniformLocation(shaderProgram, "view");
    uniformLocations["modelview"] = glGetUniformLocation(shaderProgram, "modelview");
    
    uniformLocations["time"] = glGetUniformLocation(shaderProgram, "time");
    uniformLocations["amplitude"] = glGetUniformLocation(shaderProgram, "amplitude");
    // TODO: insert the uniform locations for all light and material properties
    // - insert then into the provided map 'uniformLocations' and give them a proper identifier
    // - when accessing a GLSL uniform within a struct (as used in the provided vertex shader),
    //   use the following technique: glGetUniformLocation(shaderID, "structName.propertyName")
    //   So, when having a struct 
    //	 struct MyStruct {vec3 MyVector};
    //   and a uniform declaration
    //     uniform MyStruct MyStructUniform;
    //   you can get the location of MyVector by passing the string "MyStructUniform.MyVector" to
    //   glGetUniformLocation(...)
    
    uniformLocations["lightSource.ambient_color"] = glGetUniformLocation(shaderProgram, "lightSource.ambient_color");
    uniformLocations["lightSource.diffuse_color"] = glGetUniformLocation(shaderProgram, "lightSource.diffuse_color");
    uniformLocations["lightSource.specular_color"] = glGetUniformLocation(shaderProgram, "lightSource.specular_color");
    uniformLocations["lightSource.position"] = glGetUniformLocation(shaderProgram, "lightSource.position");

    uniformLocations["material.ambient_color"] = glGetUniformLocation(shaderProgram, "material.ambient_color");
    uniformLocations["material.diffuse_color"] = glGetUniformLocation(shaderProgram, "material.diffuse_color");
    uniformLocations["material.specular_color"] = glGetUniformLocation(shaderProgram, "material.specular_color");
    uniformLocations["material.specular_shininess"] = glGetUniformLocation(shaderProgram, "material.specular_shininess");
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
    std::string shaderCode;
    std::ifstream shaderStream(fileName, std::ios::in);
    if(shaderStream.is_open()){
        std::string line = "";
        while(std::getline(shaderStream, line))
            shaderCode += "\n" + line;
        shaderStream.close();
    }
    else {
        printf("Impossible to open %s. Please check your directories !\n", fileName);
        return 0;
    }
    char const * shaderSrc = shaderCode.c_str();
    /*std::cout << "=================================" << std::endl;
  if (shaderType == GL_VERTEX_SHADER) {
      std::cout << "VERTEX SHADER CODE " << std::endl;
  }
  else if (shaderType == GL_FRAGMENT_SHADER) {
      std::cout << "FRAGMENT SHADER CODE " << std::endl;
  }
  std::cout << shaderSrc << std::endl;
  std::cout << "=================================\n\n\n" << std::endl;*/
    
    if (shaderSrc == NULL) return 0;
    // pass source code to new shader object //
    glShaderSource(shader, 1, &shaderSrc, NULL);
    
    // compile shader //
    glCompileShader(shader);
    
    // log compile messages, if any //
    printShaderInfoLog(shader);
    
    // return compiled shader (may have compiled WITH errors) //
    return shader;
}

void initScene() {
    // load armadillo.obj from disk and create renderable MeshObj //
    objLoader->loadObjFile("meshes/armadillo.obj", "armadillo");
    
    // init materials //
    // TODO: initialize your materials here //
    // - create a new Material and insert it into the 'materials' vector
    // - set material properties for ambient, diffuse and specular color as glm::vec3
    // - set shininess exponent as float
    
    materials.push_back({{1, 0, 0}, {1, 0, 0}, {1, 1, 1}, 15});
    materials.push_back({{0, 1, 0}, {0, 1, 0}, {1, 1, 1}, 40});
    materials.push_back({{0, 0, 1}, {0, 0, 1}, {1, 1, 1}, 100});
    materials.push_back({{1, 1, 0}, {1, 1, 0}, {1, 1, 1}, 5});
    
    // save material count for later and select first material //
    materialCount = materials.size();
    materialIndex = 0;
    currentMaterial = &materials[materialIndex];
    
    // init lights //
    // TODO: initialize your light sources here //
    // - set the color properties of the light source as glm::vec3
    // - set the lights position as glm::vec3
    
    lights.push_back({{.1, .1, .1}, {1, 1, 1}, {1, 1, 1}, {4, -4, 4}});
    lights.push_back({{0, 0, .4}, {.1, .1, .8}, {.1, .1, .8}, {0, 2, 0}});
    
    // save light source count for later and select first light source //
    lightCount = lights.size();
    lightIndex = 0;
    currentLight = &lights[lightIndex];
    
    
}

void renderScene() {
    glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());
    glm_ModelViewMatrix.top() *= glm::translate(glm::vec3(0.0, -0.1, 0.0));
    glm_ModelViewMatrix.top() *= glm::scale(glm::vec3(0.2));
    glm_ModelViewMatrix.top() *= glm::rotate(GLfloat(180.0),glm::vec3(0.0, 1.0, 0.0));
    
    glUniformMatrix4fv(uniformLocations["modelview"], 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
    glUniformMatrix4fv(uniformLocations["view"], 1, false, glm::value_ptr(camera.getModelViewMat()));
    
    glm::vec3 position = glm::vec3(glm::rotateY(currentLight->position, rotAngle));
    
    // TODO: upload the properties of the currently chosen light source here //
    // - ambient, diffuse and specular color
    // - position
    // - use glm::value_ptr() to get a proper reference when uploading the values as a data vector //
    glUniform3fv(uniformLocations["lightSource.ambient_color"], 1, glm::value_ptr(currentLight->ambient_color));
    glUniform3fv(uniformLocations["lightSource.diffuse_color"], 1, glm::value_ptr(currentLight->diffuse_color));
    glUniform3fv(uniformLocations["lightSource.specular_color"], 1, glm::value_ptr(currentLight->specular_color));
    glUniform3fv(uniformLocations["lightSource.position"], 1, glm::value_ptr(position));
    
    // TODO: upload the chosen material properties here //
    // - upload ambient, diffuse and specular color as 3d-vector
    // - upload shininess exponent as simple float value
    glUniform3fv(uniformLocations["material.ambient_color"], 1, glm::value_ptr(currentMaterial->ambient_color));
    glUniform3fv(uniformLocations["material.diffuse_color"], 1, glm::value_ptr(currentMaterial->diffuse_color));
    glUniform3fv(uniformLocations["material.specular_color"], 1, glm::value_ptr(currentMaterial->specular_color));
    glUniform1f(uniformLocations["material.specular_shininess"], currentMaterial->specular_shininess);
    
    // TODO: upload pumping properties here //
    glUniform1f(uniformLocations["time"], timeValue);
    glUniform1f(uniformLocations["amplitude"], pumpAmplitude);
    
    // render the actual object //
    objLoader->getMeshObj("armadillo")->render();
    
    // restore scene graph to previous state //
    glm_ModelViewMatrix.pop();
}

void updateGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // set viewport dimensions //
    GLint dim = glm::min(windowWidth - guiWidth, windowHeight);
    glViewport(0, 0, dim, dim);
    
    // get projection mat from camera controller //
    glm_ProjectionMatrix.top() = camera.getProjectionMat();
    // upload projection matrix //
    glUniformMatrix4fv(uniformLocations["projection"], 1, false, glm::value_ptr(glm_ProjectionMatrix.top()));
    
    // init scene graph by cloning the top entry, which can now be manipulated //
    // get modelview mat from camera controller //
    glm_ModelViewMatrix.top() = camera.getModelViewMat();
    
    // render scene //
    renderScene();
    
    // TODO : increment time parameter
    if(pumpAnim) {
        timeValue += pumpSpeed;
        uniformLocations["time"];
    }
    
    if (rotAnim) {
        // TODO : increment rotation angle //
        rotAngle += 1;
        
        
        
        if (rotAngle > 360.0f) rotAngle -= 360.0f;
    }
    
    // Draw GUI //
    TwDraw();
    
    // swap renderbuffers for smooth rendering //
    glutSwapBuffers();
}

void idle() {
    glutPostRedisplay();
}

void keyboardEvent(unsigned char key, int x, int y) {
    
    if (TwEventKeyboardGLUT(key, x, y) != 1) { // if GUI has not responded to event
        switch (key) {
        case 'x':
        case 27 : {
            exit(0);
            break;
        }
        case 'w': {
            // move forward //
            camera.move(CameraController::MOVE_FORWARD);
            break;
        }
        case 's': {
            // move backward //
            camera.move(CameraController::MOVE_BACKWARD);
            break;
        }
        case 'a': {
            // move left //
            camera.move(CameraController::MOVE_LEFT);
            break;
        }
        case 'd': {
            // move right //
            camera.move(CameraController::MOVE_RIGHT);
            break;
        }
        case 'r': {
            // move up //
            camera.move(CameraController::MOVE_UP);
            break;
        }
        case 'f': {
            // move down //
            camera.move(CameraController::MOVE_DOWN);
            break;
        }
        case 'm': {
            materialIndex++;
            if (materialIndex >= materialCount) materialIndex = 0;
            currentMaterial = &materials[materialIndex];
            updateBar();
            break;
        }
        case 'l': {
            lightIndex++;
            if (lightIndex >= lightCount) lightIndex = 0;
            currentLight = &lights[lightIndex];
            updateBar();
            break;
        }
        }
    }
    glutPostRedisplay();
}

void mouseEvent(int button, int state, int x, int y) {
    if (TwEventMouseButtonGLUT(button, state, x, y) != 1)
    { // if GUI has not responded to event
        CameraController::MouseState mouseState;
        if (state == GLUT_DOWN) {
            switch (button) {
            case GLUT_LEFT_BUTTON : {
                mouseState = CameraController::LEFT_BTN;
                break;
            }
            case GLUT_RIGHT_BUTTON : {
                mouseState = CameraController::RIGHT_BTN;
                break;
            }
            default : break;
            }
        } else {
            mouseState = CameraController::NO_BTN;
        }
        camera.updateMouseBtn(mouseState, x, y);
    }
    
    glutPostRedisplay();
}

void mouseMoveEvent(int x, int y) {
    camera.updateMousePos(x, y);
    glutPostRedisplay();
}


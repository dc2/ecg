#include "Ex06.h"
#include <sstream>

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

// this struct helps to keep light source parameter uniforms together //
struct UniformLocation_Light {
    GLint ambient_color;
    GLint diffuse_color;
    GLint specular_color;
    GLint position;
};
// this map stores the light source uniform locations as 'UniformLocation_Light' structs //
std::map<std::string, UniformLocation_Light> uniformLocations_Lights;

// these structs are also used in the shader code  //
// this helps to access the parameters more easily //
struct Material {
    glm::vec3 ambient_color;
    glm::vec3 diffuse_color;
    glm::vec3 specular_color;
    float specular_shininess;
};

struct LightSource {
    //LightSource() : enabled(false) {};
    bool enabled;
    glm::vec3 ambient_color;
    glm::vec3 diffuse_color;
    glm::vec3 specular_color;
    glm::vec3 position;
};

// the program uses a list of materials and light sources, which can be chosen during rendering //
unsigned int materialIndex;
unsigned int materialCount;
std::vector<Material> materials;
unsigned int lightCount;
std::vector<LightSource> lights;

// window controls //
void updateBar();
void updateGL();
void idle();
void close();
void Reshape(int width, int height);
void keyboardEvent(unsigned char key, int x, int y);
void mouseEvent(int button, int state, int x, int y);
void mouseMoveEvent(int x, int y);

// camera controls //
CameraController camera(0, 0, 1.0);			// camera looks frontal on scene, distance from origin = 1.0

// viewport //
TwBar * CameraGUI;
GLint windowWidth, windowHeight;
GLint guiWidth;

// geometry //
void initScene();
void renderScene();

// OBJ import //
ObjLoader *objLoader = 0;

int main (int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion(3,3);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    
    
    windowHeight = 512;
    guiWidth = 300;
    windowWidth = windowHeight + guiWidth;
    
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Exercise 06 - Multiple Light Sources");
    
    glutDisplayFunc(updateGL);
    glutReshapeFunc(Reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboardEvent);
    glutMouseFunc(mouseEvent);
    glutMotionFunc(mouseMoveEvent);
    glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);
    TwGLUTModifiersFunc(glutGetModifiers);
    glutCloseFunc(close);
    
    // Initialize the GUI
    TwInit(TW_OPENGL_CORE, NULL);
    TwWindowSize(windowWidth, windowHeight);
    CameraGUI = TwNewBar("CameraGUI");
    TwDefine("CameraGUI refresh=0.1");
    TwDefine(" GLOBAL help='Exercise Sheet 05 - Multiple Lights.' "); // Message added to the help bar.
    
    // set position and size of AntTweakBar
    char *setbarposition = new char[1000];
    sprintf(setbarposition, " CameraGUI position='%i %i' size='%i %i'\0", windowWidth-(guiWidth-10), 20, guiWidth-20, windowHeight-40);
    std::cout << setbarposition << std::endl;
    TwDefine(setbarposition);
    delete[] setbarposition;
    
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
    camera.setAspect(1.0);
    
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
    
    for (int lidx = 0; lidx < lights.size(); lidx++) {
        //std::stringstream label;
        //label << " group='Light " << lidx+1 << "' ";
        std::stringstream name;
        name << "Light " << lidx+1;
        TwAddVarRW(CameraGUI, name.str().c_str(), TW_TYPE_BOOL8 , &lights[lidx].enabled, "help='Light enabled?'");
    }
    
    /*
      
    std::stringstream label;
    
    label << " group='Material " << materialIndex << "' ";
    TwAddVarRW(CameraGUI, "Ambient M", TW_TYPE_COLOR3F, &currentMaterial->ambient_color, label.str().c_str());
    TwAddVarRW(CameraGUI, "Diffuse M", TW_TYPE_COLOR3F, &currentMaterial->diffuse_color, label.str().c_str());
    TwAddVarRW(CameraGUI, "Specular M", TW_TYPE_COLOR3F, &currentMaterial->specular_color, label.str().c_str());
    TwAddVarRW(CameraGUI, "Shininess M", TW_TYPE_FLOAT, &currentMaterial->specular_shininess, label.str().c_str());
    
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
    label2 << 
    TwAddVarRW(CameraGUI, "Rotating Light", TW_TYPE_BOOL8 , &rotAnim, "help='Auto-rotate light?'");
    TwAddVarRW(CameraGUI, "Rotation Angle", TW_TYPE_FLOAT, &rotAngle, "step=1.0");
    TwAddVarRW(CameraGUI, "Pumping Animation", TW_TYPE_BOOL8 , &pumpAnim, "help='Pumping animation?'");
    TwAddVarRW(CameraGUI, "Pumping Speed", TW_TYPE_FLOAT, &pumpSpeed, "step=0.01");
    TwAddVarRW(CameraGUI, "Pumping Amplitude", TW_TYPE_FLOAT, &pumpAmplitude, "step=0.001");
    
    */
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

std::string getUniformStructLocStr(const std::string &structName, const std::string &memberName, int arrayIndex = -1) {
    std::stringstream sstr("");
    sstr << structName;
    if (arrayIndex >= 0) {
        sstr << "[" << arrayIndex << "]";
    }
    sstr << "." << memberName;
    return sstr.str();
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
    printProgramInfoLog(shaderProgram);
    
    // set address of fragment color output //
    glBindFragDataLocation(shaderProgram, 0, "color");
    
    // get uniform locations for common variables //
    uniformLocations["projection"] = glGetUniformLocation(shaderProgram, "projection");
    uniformLocations["modelview"] = glGetUniformLocation(shaderProgram, "modelview");
    uniformLocations["view"] = glGetUniformLocation(shaderProgram, "view");
    
    // material unform locations //
    uniformLocations["material.ambient"] = glGetUniformLocation(shaderProgram, "material.ambient_color");
    uniformLocations["material.diffuse"] = glGetUniformLocation(shaderProgram, "material.diffuse_color");
    uniformLocations["material.specular"] = glGetUniformLocation(shaderProgram, "material.specular_color");
    uniformLocations["material.shininess"] = glGetUniformLocation(shaderProgram, "material.specular_shininess");
    
    // store the uniform locations for all light source properties
    // - create a 'UniformLocation_Light' struct to store the light source parameter uniforms 
    // - insert this strut into the provided map 'uniformLocations_Lights' and give it a proper name
    for(int i=0; i<10; ++i)
    {
        uniformLocations_Lights[std::to_string(i)].ambient_color = glGetUniformLocation(shaderProgram, std::string("lights["+std::to_string(i)+"].ambient_color").c_str());
        uniformLocations_Lights[std::to_string(i)].diffuse_color = glGetUniformLocation(shaderProgram, std::string("lights["+std::to_string(i)+"].diffuse_color").c_str());
        uniformLocations_Lights[std::to_string(i)].specular_color = glGetUniformLocation(shaderProgram, std::string("lights["+std::to_string(i)+"].specular_color").c_str());
        uniformLocations_Lights[std::to_string(i)].position = glGetUniformLocation(shaderProgram, std::string("lights["+std::to_string(i)+"].position").c_str());
        //std::cout << uniformLocations_Lights[std::to_string(i)].ambient_color << std::endl;
    }
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
    // load scene.obj from disk and create renderable MeshObj //
    std::cout << "loading mesh....." << std::endl;
    objLoader->loadObjFile("meshes/armadillo.obj", "model");
    
    // init materials //
    // - create a new Material and insert it into the 'materials' vector
    // - set material properties for ambient, diffuse and specular color as glm::vec3
    // - set shininess exponent as float
    Material mat;
    mat.ambient_color = glm::vec3(0);
    mat.diffuse_color = glm::vec3(1);
    mat.specular_color = glm::vec3(.5);
    mat.specular_shininess = 50.0;
    materials.push_back(mat);
    
    mat.ambient_color = glm::vec3(1.0, 0.0, 0.0);
    mat.diffuse_color = glm::vec3(1.0, 0.0, 0.0);
    mat.specular_color = glm::vec3(1.0, 1.0, 1.0);
    mat.specular_shininess = 15.0;
    materials.push_back(mat);
    
    mat.ambient_color = glm::vec3(0.0, 1.0, 0.0);
    mat.diffuse_color = glm::vec3(0.0, 1.0, 0.0);
    mat.specular_color = glm::vec3(1.0, 1.0, 1.0);
    mat.specular_shininess = 50.0;
    materials.push_back(mat);
    
    mat.ambient_color = glm::vec3(0.0, 0.0, 1.0);
    mat.diffuse_color = glm::vec3(0.0, 0.0, 1.0);
    mat.specular_color = glm::vec3(1.0, 1.0, 1.0);
    mat.specular_shininess = 150.0;
    materials.push_back(mat);
    
    mat.ambient_color = glm::vec3(1.0, 0.8, 0.3);
    mat.diffuse_color = glm::vec3(1.0, 0.8, 0.3);
    mat.specular_color = glm::vec3(1.0, 1.0, 1.0);
    mat.specular_shininess = 5.0;
    materials.push_back(mat);
    
    // save material count for later and select first material //
    materialCount = materials.size();
    materialIndex = 0;
    
    // init lights //
    // initialize your light sources here //
    // - set the color properties of the light source as glm::vec3
    // - set the lights position as glm::vec3
    // - create up to 10 light sources (you may toggle them later on using the keys '0' through '9')
    for(int i=0; i<10; ++i)
    {
        lights.push_back({false, {.05, .05, .05}, {(i%3==0), (i%3==1), (i%3==2)}, {1, 1, 1}, {10*(i%2)-5+i, 10*(i%4)-5, 3*i - 1.5}});
    }
    
    
    // save light source count for later and select first light source //
    lightCount = lights.size();
    
    lights[0].enabled = true;
}

void renderScene() {
    glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());
    
    glm_ModelViewMatrix.top() *= glm::translate(glm::vec3(0.0, -0.1, 0.0));
    glm_ModelViewMatrix.top() *= glm::scale(glm::vec3(0.2));
    glm_ModelViewMatrix.top() *= glm::rotate(GLfloat(180.0),glm::vec3(0.0, 1.0, 0.0));
    
    glUniformMatrix4fv(uniformLocations["modelview"], 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
    glUniformMatrix4fv(uniformLocations["view"], 1, false, glm::value_ptr(camera.getModelViewMat()));
    
    // upload the properties of the currently active light sources here //
    // - ambient, diffuse and specular color
    // - position
    // - use glm::value_ptr() to get a proper reference when uploading the values as a data vector //
    for(int i=0; i<10; ++i)
    {
        if(lights[i].enabled)
        {
            glUniform3fv(uniformLocations_Lights[std::to_string(i)].ambient_color, 1, glm::value_ptr(lights[i].ambient_color));
            glUniform3fv(uniformLocations_Lights[std::to_string(i)].diffuse_color, 1, glm::value_ptr(lights[i].diffuse_color));
            glUniform3fv(uniformLocations_Lights[std::to_string(i)].specular_color, 1, glm::value_ptr(lights[i].specular_color));
            glUniform3fv(uniformLocations_Lights[std::to_string(i)].position, 1, glm::value_ptr(lights[i].position));
        } else {
            glUniform3fv(uniformLocations_Lights[std::to_string(i)].ambient_color, 1, glm::value_ptr(glm::vec3(0)));
            glUniform3fv(uniformLocations_Lights[std::to_string(i)].diffuse_color, 1, glm::value_ptr(glm::vec3(0)));
            glUniform3fv(uniformLocations_Lights[std::to_string(i)].specular_color, 1, glm::value_ptr(glm::vec3(0)));
            glUniform3fv(uniformLocations_Lights[std::to_string(i)].position, 1, glm::value_ptr(glm::vec3(0)));
        }
    }
    
    
    // upload the chosen material properties here //
    // - upload ambient, diffuse and specular color as 3d-vector
    // - upload shininess exponent as simple float value
    glUniform3fv(uniformLocations["material.ambient"], 1, glm::value_ptr(materials[materialIndex].ambient_color));
    glUniform3fv(uniformLocations["material.diffuse"], 1, glm::value_ptr(materials[materialIndex].diffuse_color));
    glUniform3fv(uniformLocations["material.specular"], 1, glm::value_ptr(materials[materialIndex].specular_color));
    glUniform1f(uniformLocations["material.shininess"], materials[materialIndex].specular_shininess);
    
    // render the actual object //
    objLoader->getMeshObj("model")->render();
    
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
    
    // Draw GUI //
    TwDraw();
    
    // swap renderbuffers for smooth rendering //
    glutSwapBuffers();
}

void idle() {
    glutPostRedisplay();
}

// toggles a light source on or off //
void toggleLightSource(unsigned int i) {
    if (i < lightCount) {
        lights[i].enabled = !lights[i].enabled;
    }
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
            break;
        }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            int lightIdx;
            std::stringstream keyStr;
            keyStr << key;
            keyStr >> lightIdx;
            if (lightIdx == 0) lightIdx = 10;
            if (lightIdx > 0) toggleLightSource(lightIdx - 1);
            break;
        }
        }
    }
    glutPostRedisplay();
}


void mouseEvent(int button, int state, int x, int y) {
    
    
    // use event in GUI
    if (TwEventMouseButtonGLUT(button, state, x, y) != 1) { // if GUI has not responded to event
        
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
    // use event in GUI
    if (TwEventMouseMotionGLUT(x, y) != 1) { // if GUI has not responded to event
        camera.updateMousePos(x, y);
    }
    glutPostRedisplay();
}

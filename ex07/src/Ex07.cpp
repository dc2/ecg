#include "Ex07.h"
#include <sstream>

#include <glm/gtc/matrix_transform.hpp>

#include "lib/ShaderProgram.h"

// OpenGL and GLSL stuff //
void initGL();
void initShader();

bool glError(char *msg);

std::unique_ptr<ShaderProgram> shaderTex;
std::unique_ptr<ShaderProgram> shaderSimple;

TextureData textureData1;
TextureData textureData2;

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
    LightSource() : enabled(true) {}
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
int CheckGLErrors();
void updateGL();
void idle();
void close();
void Reshape(int width, int height);
void keyboardEvent(unsigned char key, int x, int y);
void mouseEvent(int button, int state, int x, int y);
void mouseMoveEvent(int x, int y);

// camera controls //
CameraController camera(0, M_PI/4, 20);

// viewport //
GLint windowWidth, windowHeight;

// geometry //
void initScene();
void renderScene();

// textures //
void initTextures();

// OBJ import //
ObjLoader *objLoader = 0;

int main (int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitContextVersion(3,3);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    
    windowWidth = 512;
    windowHeight = 512;
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Exercise 07 - Textures");
    
    glutDisplayFunc(updateGL);
    glutReshapeFunc(Reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboardEvent);
    glutMouseFunc(mouseEvent);
    glutMotionFunc(mouseMoveEvent);
    glutCloseFunc(close);
    
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cout << "(glewInit) - Error: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "(glewInit) - Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
    
    // init stuff //
    initGL();
    
    // init object loader
    objLoader = new ObjLoader();
    
    // init matrix stacks with identity //
    glm_ProjectionMatrix.push(glm::mat4(1));
    glm_ModelViewMatrix.push(glm::mat4(1));
    
    shaderTex = ShaderProgram::createFromFiles("shader/texture.vert", "shader/texture.frag");
    shaderSimple = ShaderProgram::createFromFiles("shader/simple.vert", "shader/simple.frag");
    initScene();
    
    textureData1.init("textures/trashbin.png");
    textureData2.init("textures/ball.jpg");
    
    shaderTex->bind();
    glutMainLoop();
    
    return 0;
}

int CheckGLErrors() {
    int errCount = 0;
    for(GLenum currError = glGetError(); currError != GL_NO_ERROR; currError = glGetError()) {
        std::stringstream sstr;
        
        switch (currError) {
        case GL_INVALID_ENUM : sstr << "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE : sstr << "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION : sstr << "GL_INVALID_OPERATION"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION : sstr << "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        case GL_OUT_OF_MEMORY : sstr << "GL_OUT_OF_MEMORY"; break;
        default : sstr << "unknown error (" << currError << ")";
        }
        std::cout << "found error: " << sstr.str() << std::endl;
        ++errCount;
    }
    return errCount;
}

void initGL() {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
}

// Callback function called by GLUT when window size changes
void Reshape(int width, int height)
{
    // Set OpenGL viewport and camera
    windowWidth = width;
    windowHeight = height;
    
}

void close() {
    // free allocated objects
    delete objLoader;
    
    std::cout << "Shutdown program." << std::endl;
}

void initScene() {
    objLoader->loadObjFile("meshes/plane2.obj", "plane");
    
    // TODO (7.4) : load trashbin and ball from disk and create renderable meshes //
    objLoader->loadObjFile("meshes/trashbin.obj", "trashbin");
    objLoader->loadObjFile("meshes/ball.obj", "ball");
    
    // TODO (7.5): load Optimus Prime and Megatron from disk and create renderable meshes //
    objLoader->loadObjFile("meshes/optimus.obj", "optimus");
    objLoader->loadObjFile("meshes/megatron.obj", "megatron");
    
    // init materials //
    Material mat;
    mat.ambient_color = glm::vec3(1.0, 1.0, 1.0);
    mat.diffuse_color = glm::vec3(1.0, 1.0, 1.0);
    mat.specular_color = glm::vec3(1.0, 1.0, 1.0);
    mat.specular_shininess = 50.0;
    materials.push_back(mat);
    
    mat.specular_shininess = 100.0;
    materials.push_back(mat);
    
    mat.ambient_color = glm::vec3(1.0, 0.9, 0.8);
    mat.diffuse_color = glm::vec3(1.0, 0.9, 0.8);
    mat.specular_shininess = 50.0;
    materials.push_back(mat);
    
    // save material count for later and select first material //
    materialCount = materials.size();
    materialIndex = 0;
    
    // init lights //
    
    LightSource light;
    light.ambient_color = glm::vec3(0.001, 0.001, 0.001);
    light.diffuse_color = glm::vec3(1.0, 1.0, 1.0);
    light.specular_color = glm::vec3(1.0, 1.0, 1.0);
    
    light.position = glm::vec3(15, 15, 15);
    lights.push_back(light);
    
    light.position = glm::vec3(-15, 15, 15);
    lights.push_back(light);
    
    light.position = glm::vec3(15, 15, -15);
    lights.push_back(light);
    
    light.position = glm::vec3(-15, 15, -15);
    lights.push_back(light);
    
    // Todo (7.5) : estimate nicer lighting situation for Transformers //
    
    // save light source count for later and select first light source //
    lightCount = lights.size();
}

void renderScene() {
    {
        shaderSimple->bind();
        glDisable(GL_DEPTH_TEST);
        
        shaderSimple->setUniform("modelview",  glm::mat4(1));
        shaderSimple->setUniform("projection", glm::mat4(1));
    
        objLoader->getMeshObj("plane")->render();
    }
    
    shaderTex->bind();
    glEnable(GL_DEPTH_TEST);
    
    // TODO: upload the properties of the currently active light sources here //
    // - ambient, diffuse and specular color
    // - position
    // - use glm::value_ptr() to get a proper reference when uploading the values as a data vector //
    // upload the properties of the currently active light sources //
    int shaderLightIdx = 0;
    for (unsigned i = 0; i < lightCount; ++i) {
        if (lights[i].enabled) {
            std::string light_name = "lightSource["+std::to_string(i)+"]";
            
            shaderTex->setUniform(light_name+".position", lights[i].position);
            shaderTex->setUniform(light_name+".ambient_color", lights[i].ambient_color);
            shaderTex->setUniform(light_name+".diffuse_color", lights[i].diffuse_color);
            shaderTex->setUniform(light_name+".specular_color", lights[i].specular_color);
            
            ++shaderLightIdx;
        }
    }

    shaderTex->setUniform("usedLightCount", shaderLightIdx);
    
    shaderTex->setUniform("material.ambient_color", materials[materialIndex].ambient_color);
    shaderTex->setUniform("material.diffuse_color", materials[materialIndex].diffuse_color);
    shaderTex->setUniform("material.specular_color", materials[materialIndex].specular_color);
    shaderTex->setUniform("material.specular_shininess", materials[materialIndex].specular_shininess);
    
    shaderTex->setUniform("tex", 1);
    
    // TODO: upload respective texture to first texture unit and render the actual scene //
    glActiveTexture(GL_TEXTURE1);
    {
        glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());
        glm_ModelViewMatrix.top() *= glm::translate(glm::vec3(3, 0, 0));
        shaderTex->setUniform("modelview", glm_ModelViewMatrix.top());
        
        glBindTexture(GL_TEXTURE_2D, textureData1.texture);
        
        objLoader->getMeshObj("megatron")->render();
        //objLoader->getMeshObj("optimus")->render();
        
        glm_ModelViewMatrix.pop();
    }
    
    {
        glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());
        glm_ModelViewMatrix.top() *= glm::translate(glm::vec3(-2, 0, 0));
        
        shaderTex->setUniform("modelview", glm_ModelViewMatrix.top());
        glBindTexture(GL_TEXTURE_2D, textureData2.texture);
        
        objLoader->getMeshObj("optimus")->render();
        
        // restore scene graph to previous state //
        glm_ModelViewMatrix.pop();
    }
}


void updateGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // set viewport dimensions //
    glViewport(0, 0, windowWidth, windowHeight);
    
    // get projection mat from camera controller //
    glm_ProjectionMatrix.top() = camera.getProjectionMat();
    // upload projection matrix //
    shaderTex->setUniform("projection", glm_ProjectionMatrix.top());
    
    // init scene graph by cloning the top entry, which can now be manipulated //
    // get modelview mat from camera controller //
    glm_ModelViewMatrix.top() = camera.getModelViewMat();
    
    // render scene //
    renderScene();
    
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
    case 'z': {
        camera.setOpeningAngle(camera.getOpeningAngle() + 0.1f);
        break;
    }
    case 'h': {
        camera.setOpeningAngle(std::min(std::max(camera.getOpeningAngle() - 0.1f, 1.0f), 180.0f));
        break;
    }
    case 'r': {
        camera.setNear(std::min(camera.getNear() + 0.1f, camera.getFar() - 0.01f));
        break;
    }
    case 'f': {
        camera.setNear(std::max(camera.getNear() - 0.1f, 0.1f));
        break;
    }
    case 't': {
        camera.setFar(camera.getFar() + 0.1f);
        break;
    }
    case 'g': {
        camera.setFar(std::max(camera.getFar() - 0.1f, camera.getNear() + 0.01f));
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
    glutPostRedisplay();
}

void mouseEvent(int button, int state, int x, int y) {
    CameraController::MouseState mouseState = CameraController::NO_BTN;
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
    glutPostRedisplay();
}

void mouseMoveEvent(int x, int y) {
    camera.updateMousePos(x, y);
    glutPostRedisplay();
}


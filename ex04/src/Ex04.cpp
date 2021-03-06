#include "Ex04.h"

// OpenGL and GLSL stuff //
void initGL();
void initShader();
bool enableShader();
void disableShader();
void deleteShader();
bool glError(char *msg);

GLuint loadShaderFile(const char* fileName, GLenum shaderType);
GLuint shaderProgram = 0;

// camera controls //
CameraController cameraView(0, 0, 1.0);            // camera looks frontal on scene, distance from origin = 1.0
CameraController sceneView(M_PI/4, M_PI/6, 4);  // second camera looks from above/side, distance from origin = 4.0

// canonical volume start orientation
glm::quat cv_Rotation = glm::quat(0.98, 0.21, 0.07, 0.0);
GLfloat cv_scale = 0.55;
GLfloat rotAngle = 0;
bool rotAnim     = true;

// viewport //
GLint windowWidth    = 1600;
GLint windowHeight    = 600;
GLint guiWidth        = 200;
GLint numViews        = 3;
GLint widthview;
enum VIEW {CAMERA_VIEW, WORLD_VIEW, CV_VIEW};

glm::mat4 invertProjectionMat(const glm::mat4 &mat);
void renderCameraView();
void renderCameraSpaceVisualization();
void renderCanonicalVolumeVisualization();

// window controls //
void updateGL();
void idle();
void Terminate(void);
void setViewport(VIEW view);
void Reshape(int width, int height);
void keyboardEvent(unsigned char key, int x, int y);
void mouseEvent(int button, int state, int x, int y);
void mouseMoveEvent(int x, int y);

// geometry //
void initScene();
void renderScene();
void deleteScene();
// cube representing camera frustum //
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
GLuint cubeIBO = 0;
// camera front side is rendered green
GLuint camSideVAO = 0;
GLuint camSideVBO = 0;
GLuint camSideIBO = 0;

int main (int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion(3,3);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    
    glutInitWindowSize (windowWidth, windowHeight);
    glutInitWindowPosition (10, 10);
    glutCreateWindow("Exercise 04 - Camera control and projection");
    glutCreateMenu(NULL);
    widthview = (windowWidth - guiWidth) / numViews;
    std::cout << "Width of a single view = " << widthview << std::endl;
    
    glutDisplayFunc(updateGL);
    glutReshapeFunc(Reshape);
    glutIdleFunc(idle);
    atexit(Terminate);  // Called after glutMainLoop ends
    
    // configure Camera
    cameraView.setFar(1.5);
    cameraView.setNear(0.5);
    cameraView.setOpeningAngle(45.0);
    cameraView.setAspect(widthview/float(windowHeight));
    
    // Initialize the GUI
    TwInit(TW_OPENGL_CORE, NULL);
    TwWindowSize(windowWidth, windowHeight);
    TwBar * CameraGUI = TwNewBar("CameraGUI");
    TwDefine(" GLOBAL help='Exercise Sheet 04 - Camera controller and projection.' "); // Message added to the help bar.
    
    // set position and size of AntTweakBar
    char *setbarposition = new char[1000];
    sprintf(setbarposition, " CameraGUI position='%i %i' size='%i %i'\0", windowWidth-(guiWidth-10), 20, guiWidth-20, windowHeight-40);
    std::cout << setbarposition << std::endl;
    TwDefine(setbarposition);
    delete[] setbarposition;
    
    // configure GUI editing bars
    TwSetParam(CameraGUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
    TwAddVarRW(CameraGUI, "zNear", TW_TYPE_FLOAT, &cameraView.mNear, "step=0.01");
    TwAddVarRW(CameraGUI, "zFar", TW_TYPE_FLOAT, &cameraView.mFar, "step=0.01");
    TwAddVarRW(CameraGUI, "OpenAngle", TW_TYPE_FLOAT, &cameraView.mOpenAngle, "step=0.01");
    TwAddVarRW(CameraGUI, "Aspect", TW_TYPE_FLOAT, &cameraView.mAspect, "step=0.01");
    TwAddVarRW(CameraGUI, "Theta", TW_TYPE_FLOAT, &cameraView.mTheta, "step=0.01");
    TwAddVarRW(CameraGUI, "Phi", TW_TYPE_FLOAT, &cameraView.mPhi, "step=0.01");
    TwAddVarRW(CameraGUI, "CV Navigation", TW_TYPE_QUAT4F, &cv_Rotation, "showval=false open=true");
    TwAddVarRW(CameraGUI, "scale CV", TW_TYPE_FLOAT, &cv_scale, "step=0.01");
    TwAddVarRW(CameraGUI, "Auto-Rotation", TW_TYPE_BOOL8 , &rotAnim, "help='Auto-rotate canonical volume?'");
    TwAddVarRW(CameraGUI, "Angle", TW_TYPE_FLOAT, &rotAngle, "step=1.0");
    
    // redirect GLUT events to AntTweakBar
    glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
    glutMouseFunc(mouseEvent);
    glutMotionFunc(mouseMoveEvent);
    glutKeyboardFunc(keyboardEvent);
    glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);
    TwGLUTModifiersFunc(glutGetModifiers);
    
    // initialize GL content
    glError("creating rendering context");
    glewExperimental = true;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cout << "(glewInit) - Error: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "(glewInit) - Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
    glError("rendering context created");
    
    // init cameras //
    sceneView.setFar(100);
    
    // init OpenGL //
    initGL();
    glError("GL initialized");
    
    // init matrix stacks //
    glm_ProjectionMatrix.push(glm::mat4(1.0));
    glm_ModelViewMatrix.push(glm::mat4(1.0));
    
    initShader();
    glError("shader initialized");
    
    
    initScene();
    glError("scene initialized");
    
    // start render loop //
    if (enableShader()) {
        glError("enableShader()");
        
        glutMainLoop();
        disableShader();
        
        // clean up allocated data //
        deleteScene();
        deleteShader();
    }
    
    return 0;
}

// Function called at exit
void Terminate(void)
{ 
    TwTerminate();
}

void setViewport(VIEW view) {
    // TODO : compute viewport size and position using enum View
    switch(view)
    {
    case VIEW::CAMERA_VIEW:
        glViewport(0, 0, widthview, windowHeight);
        break;
        
    case VIEW::WORLD_VIEW:
        glViewport(widthview, 0, widthview, windowHeight);
        break;
        
    default:
    case CV_VIEW:
        glViewport(2*widthview, 0, widthview, windowHeight);
        break;
    }
}

// Callback function called by GLUT when window size changes
void Reshape(int width, int height)
{
    // Set OpenGL viewport and camera
    windowWidth = width;
    windowHeight = height;
    widthview = (windowWidth - guiWidth) / numViews;
    
    if(width > 0 && height > 0)
    {
        cameraView.setAspect(widthview / float(windowHeight));
        sceneView.setAspect(widthview / float(windowHeight));
    }
    
    // Send the new window size to AntTweakBar
    TwWindowSize(windowWidth, windowHeight);
    
    // set position and size of AntTweakBar
    char *setbarposition = new char[1000];
    sprintf(setbarposition, " CameraGUI position='%i %i' size='%i %i'\0", windowWidth-(guiWidth-10), 20, guiWidth-20, windowHeight-40);
    std::cout << setbarposition << std::endl;
    TwDefine(setbarposition);
    delete[] setbarposition;
    
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
    
    GLuint vertexShader = loadShaderFile("shader/ndc.vert", GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        std::cout << "(initShader) - Could not create vertex shader." << std::endl;
        deleteShader();
        return;
    }
    glError("loadShaderFile(GL_VERTEX_SHADER)");
    
    GLuint fragmentShader = loadShaderFile("shader/ndc.frag", GL_FRAGMENT_SHADER);
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
    std::cout << "=================================" << std::endl;
    if (shaderType == GL_VERTEX_SHADER) {
        std::cout << "VERTEX SHADER CODE " << std::endl;
    }
    else if (shaderType == GL_FRAGMENT_SHADER) {
        std::cout << "FRAGMENT SHADER CODE " << std::endl;
    }
    std::cout << shaderSrc << std::endl;
    std::cout << "=================================\n\n\n" << std::endl;
    
    if (shaderSrc == NULL) return 0;
    // pass source code to new shader object //
    glShaderSource(shader, 1, &shaderSrc, NULL);
    
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


// compute inverse of non-affine matrices
glm::mat4 invertProjectionMat(const glm::mat4 &mat) {
    
    // This method returns the inverse of a matrix, that is not affine (like OpenGLs projection matrix).
    // Input parameter  : Reference to the matrix to be inverted.
    // Output parameter : Inverse of provided matrix.
    
    glm::mat4 inv(0);
    inv[0][0] = 1 / mat[0][0];
    inv[1][1] = 1 / mat[1][1];
    inv[2][2] = mat[3][3];
    inv[2][3] = 1 / mat[3][2];
    inv[3][2] = mat[2][3];
    inv[3][3] = mat[2][2] / mat[3][2];
    return inv;
}

ObjLoader objLoader;

void initScene() {
    
    //objLoader.loadObjFile("meshes/armadillo.obj", "armadillo");
    objLoader.loadObjFile("meshes/bunny.obj", "bunny");
    objLoader.loadObjFile("meshes/camera.obj", "camera");
    
    // init frustum cube //
    GLfloat frustrumVertices[24] = {-1,-1,-1,
                                    1,-1,-1,
                                    1, 1,-1,
                                    -1, 1,-1,
                                    -1,-1, 1,
                                    1,-1, 1,
                                    1, 1, 1,
                                    -1, 1, 1};
    // define indices for cube
    GLuint frustumIndices[24] = {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        1, 5,
        2, 6,
        3, 7};
    
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);
    
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, 24*sizeof(GLfloat), frustrumVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    
    glGenBuffers(1, &cubeIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIBO);
    
    // TODO: copy data into the IBO //
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24*sizeof(GLint), frustumIndices, GL_STATIC_DRAW);
    
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //glBindVertexArray(0);
    
    // TODO : create and fill buffers for cube, upload data
    //    - cubeVAO
    //    - cubeVBO
    //    - cubeIBO
    //    - unbind vertex array at the end
    
    // TODO : define two triangles on camera front side (see figure in the exercise PDF),
    //    - create index array defining the triangles (array "camSideIndices")
    //    - create and fill buffers for triangles
    //        - camSideVAO
    //        - camSideVBO (use frustumVertices array also used for cube)
    //        - camSideIBO
    //    - unbind vertex array at the end
    
    // TODO : define two triangles on camera front side (see figure in the exercise PDF),
    //	- create index array defining the triangles (array "camSideIndices")
    //	- create and fill buffers for triangles
    //		- camSideVAO
    //		- camSideVBO (use frustumVertices array also used for cube)
    //		- camSideIBO
    GLuint camSideIndices[6] = {
        0, 1, 2,
        2, 3, 0
    };
  
    glGenVertexArrays(1, &camSideVAO);
    glBindVertexArray(camSideVAO);
  
    glGenBuffers(1, &camSideVBO);
    glBindBuffer(GL_ARRAY_BUFFER, camSideVBO);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), frustrumVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    
    glGenBuffers(1, &camSideIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, camSideIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), camSideIndices, GL_STATIC_DRAW);
    
    // unbind buffers //
    
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //glBindVertexArray(0);
    
}

void deleteScene() {
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &cubeIBO);
    glDeleteVertexArrays(1, &camSideVAO);
    glDeleteBuffers(1, &camSideVBO);
    glDeleteBuffers(1, &camSideIBO);
}

void renderScene() {
    // init scene graph by cloning the top entry, which can now be manipulated //
    glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());
    //glm_ModelViewMatrix.top() *= glm::rotate(rotAngle, glm::vec3(0, 1, 0));
    
    GLfloat rot_armadillo_y[] = {45.f, -45.f, 135.f, -135.f};
    GLfloat rot_bunny_y[] = {85.f, -5.f, 175.f, -95.f};
    
    for (int y = -1; y <= 1; y+=2) {
        for (int x = -1; x <= 1; x+=2) {
            /*// armadillo
            glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());
                    
            glm_ModelViewMatrix.top() *= glm::translate(glm::vec3((GLfloat)x * 0.15, 0.0, (GLfloat)y * 0.15));
            glm_ModelViewMatrix.top() *= glm::scale(glm::vec3(0.1, 0.1, 0.1));
            glm_ModelViewMatrix.top() *= glm::rotate(rot_armadillo_y[glm::max(y,0)*2+glm::max(x,0)], glm::vec3(0,1,0));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
            objLoader.getMeshObj("armadillo")->render();
            glm_ModelViewMatrix.pop();*/
            // bunny
            glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());
            glm_ModelViewMatrix.top() *= glm::translate(glm::vec3((GLfloat)x * 0.35, 0.0, (GLfloat)y * 0.35));
            glm_ModelViewMatrix.top() *= glm::scale(glm::vec3(0.1, 0.1, 0.1));
            glm_ModelViewMatrix.top() *= glm::rotate(rot_bunny_y[glm::max(y,0)*2+glm::max(x,0)], glm::vec3(0,1,0));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
            objLoader.getMeshObj("bunny")->render();
            glm_ModelViewMatrix.pop();
        }
    }
    
    // restore scene graph to previous state //
    glm_ModelViewMatrix.pop();
}

void renderCamera(glm::mat4 modelview) {
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(modelview));
    objLoader.getMeshObj("camera")->render();
}

void renderFrustum(glm::mat4 modelview) {
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(modelview));
    
    glUniform3f(glGetUniformLocation(shaderProgram, "override_color"), 1, 1, 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "use_override_color"), true);
    glBindVertexArray(cubeVAO);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUniform1i(glGetUniformLocation(shaderProgram, "use_override_color"), false);
}

void renderCameraSide() {
    glUniform3f(glGetUniformLocation(shaderProgram, "override_color"), 0, 1, 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "use_override_color"), true);
    glBindVertexArray(camSideVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUniform1i(glGetUniformLocation(shaderProgram, "use_override_color"), false);
}

// camera perspective
void renderCameraView() {
    // TODO : set viewport to left third of the window using setViewport(...) //
    setViewport(VIEW::CAMERA_VIEW);
    
    // TODO : set the correct projection matrix to uniform variable "projection"
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, false, glm::value_ptr(cameraView.getProjectionMat()));
    
    // TODO : set post transformation to identity in shader (uniform variable "cv_transform")
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cv_transform"), 1, false, glm::value_ptr(glm::mat4(1)));
    
    // TODO : get model view matrix from the camera and set it on top of model view stack
    glm_ModelViewMatrix.push(cameraView.getModelViewMat());
    
    // TODO : render scene
    renderScene();
}

// camera frustum in world space
void renderCameraSpaceVisualization() {
    
    // TODO: set viewport to middle third of the window using setViewport(...) //
    setViewport(VIEW::WORLD_VIEW);
    
    // TODO : set post transformation to identity in shader (uniform variable "cv_transform")
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cv_transform"), 1, false, glm::value_ptr(glm::mat4(1)));
    
    // TODO : set the correct projection matrix to uniform variable "projection"
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, false, glm::value_ptr(sceneView.getProjectionMat()));

    // TODO : get model view matrix from the second camera and set it on top of model view stack
    glm_ModelViewMatrix.push(sceneView.getModelViewMat());
    
    // TODO : render the scene
    renderScene();
    
    // TODO : render the camera model
    glm::mat4 modelview = glm_ModelViewMatrix.top();
    modelview *= (glm::affineInverse(cameraView.getModelViewMat()));
    modelview *= (glm::scale(glm::vec3(0.1)));
    renderCamera(modelview);
    
    // TODO : render the camera frustum
    modelview = glm_ModelViewMatrix.top();
    modelview *= glm::affineInverse(cameraView.getModelViewMat());
    modelview *= invertProjectionMat(cameraView.getProjectionMat());
    
    renderFrustum(modelview);
}

// projected camera frustum (canonical view)
void renderCanonicalVolumeVisualization() {
    // TODO: set viewport to right third of the window //
    setViewport(CV_VIEW);
    
    // TODO : VISUALIZE RENDERED SCENE IN CANONICAL VOLUME
    glm_ProjectionMatrix.top() = cameraView.getProjectionMat();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, false, glm::value_ptr(glm_ProjectionMatrix.top()));
    
    glm::mat4 cv_rotationMat = glm::mat4_cast(glm::conjugate(cv_Rotation));
    glm::mat4 cv_transform = glm::scale(glm::vec3(cv_scale)) * cv_rotationMat * glm::rotate(rotAngle, glm::vec3(0, 1, 0));

    // TODO : send "cv_transform" to the shader program
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cv_transform"), 1, false, glm::value_ptr(cv_transform));

    // TODO: render the scene with manually defined clipping planes
    glEnable(GL_CLIP_DISTANCE0);
    glUniform1fv(glGetUniformLocation(shaderProgram, "clip_plane_distance"), 1, &cv_scale);

    glm_ModelViewMatrix.top() = cameraView.getModelViewMat();
    renderScene();

    // disable clipping distance
    glDisable(GL_CLIP_DISTANCE0);
    
    // TODO : render frustum
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, false, glm::value_ptr(glm::mat4(1)));
    renderFrustum(glm::mat4(1));
    
    // TODO : draw two triangles defining the camera side
    renderCameraSide();

    // TODO : render camera
    glm::mat4 camera = glm::translate(glm::vec3(0, 0, -1.5)); // translate the camera in front of the clipping plane
    camera *= glm::scale(glm::vec3(.1));
    camera *= glm::rotate(180.f, glm::vec3(0, 1, 0)); // turn the camera right around
    
    renderCamera(camera);
}

void updateGL() {
    
    // clear buffer, enable transparency
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );
    
    // Draw views //
    renderCameraView();
    renderCameraSpaceVisualization();
    renderCanonicalVolumeVisualization();
    
    // increment rotation angle //
    if (rotAnim == true)
        rotAngle += 0.2f;
    if (rotAngle > 360.0f) rotAngle -= 360.0f;
    
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
        
        // check interaction in views
        CameraController *camera = 0;
        // camera view
        if ((x >= 0) && (x < (widthview * (WORLD_VIEW)))) {
            camera = &cameraView;
        }
        // world view
        else if ((x >= (widthview * WORLD_VIEW)) && (x < (widthview * (WORLD_VIEW+1)))) {
            camera = &sceneView;
        }
        // CV view not interactive at the moment
        // ...
        
        // move camera in respective view
        if (camera != 0) {
            switch (key) {
            case 'x':
            case 27 : {
                exit(0);
                break;
            }
            case 'w': {
                // move forward //
                camera->move(CameraController::MOVE_FORWARD);
                break;
            }
            case 's': {
                // move backward //
                camera->move(CameraController::MOVE_BACKWARD);
                break;
            }
            case 'a': {
                // move left //
                camera->move(CameraController::MOVE_LEFT);
                break;
            }
            case 'd': {
                // move right //
                camera->move(CameraController::MOVE_RIGHT);
                break;
            }
            case 'r': {
                // move up //
                camera->move(CameraController::MOVE_UP);
                break;
            }
            case 'f': {
                // move down //
                camera->move(CameraController::MOVE_DOWN);
                break;
            }
            }
        }
    }
    
    glutPostRedisplay();
}

void mouseEvent(int button, int state, int x, int y) {
    
    // use event in GUI
    if (TwEventMouseButtonGLUT(button, state, x, y) != 1) { // if GUI has not responded to event
        
        // check interaction in views
        CameraController *camera = 0;
        // camera view
        if ((x >= 0) && (x < (widthview * (WORLD_VIEW)))) {
            camera = &cameraView;
        }
        // world view
        else if ((x >= (widthview * WORLD_VIEW)) && (x < (widthview * (WORLD_VIEW+1)))) {
            camera = &sceneView;
        }
        
        // CV view not interactive at the moment
        // ...
        
        // move camera in respective view
        if (camera != 0) {
            
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
            camera->updateMouseBtn(mouseState, x, y);
            
        }
    }
    
    glutPostRedisplay();
}

void mouseMoveEvent(int x, int y) {
    
    // use event in GUI
    if (TwEventMouseMotionGLUT(x, y) != 1) { // if GUI has not responded to event
        
        // check interaction in views
        CameraController *camera = 0;
        // camera view
        if ((x >= 0) && (x < (widthview * (WORLD_VIEW)))) {
            camera = &cameraView;
        }
        // world view
        else if ((x >= (widthview * WORLD_VIEW)) && (x < (widthview * (WORLD_VIEW+1)))) {
            camera = &sceneView;
        }
        // CV view not interactive at the moment
        // ...
        
        // move camera in respective view
        if (camera != 0) {
            camera->updateMousePos(x, y);
        }
    }
    
    glutPostRedisplay();
}


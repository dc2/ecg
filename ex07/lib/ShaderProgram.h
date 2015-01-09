#pragma once

#include "Shader.h"

#include <unordered_set>
#include <stdexcept>
#include <unordered_map>


#include <glm/glm.hpp>



/**
 * @brief Manages GLSL Shader Programs
 */

class ShaderProgram
{
public:
    ShaderProgram();
    explicit ShaderProgram(const std::unordered_set<std::shared_ptr<Shader> > &shaders);
    
    static std::unique_ptr<ShaderProgram> create();
    static std::unique_ptr<ShaderProgram> createFromFiles(const std::string &vertexFileName, const std::string &fragmentFileName);
    static std::unique_ptr<ShaderProgram> create(std::unordered_set<std::shared_ptr<Shader>> shaders);
    
    virtual ~ShaderProgram();
    
    bool link();
    std::string getDebugLog();
    
    bool bind();
    void unbind();
    
    std::shared_ptr<Shader> attachShader(std::shared_ptr<Shader> shader);
    std::shared_ptr<Shader> attachShader(Shader::ShaderType type, const std::string &shaderSource, bool fromFile = true);
    void attachShadersFromFile(const std::string &vertexFileName, const std::string &fragmentFileName);
    
    std::shared_ptr<Shader> attachVertexShader(const std::string &shaderSource, bool fromFile = true);
    std::shared_ptr<Shader> attachFragmentShader(const std::string &shaderSource, bool fromFile = true);
    
    void detachShader(std::shared_ptr<Shader> shader);
    
    void setUniform(const std::string &name, GLint val);
    void setUniform(const std::string &name, GLfloat val);
    
    void setUniform(const std::string &name, glm::ivec2 val);
    void setUniform(const std::string &name, glm::ivec3 val);
    
    void setUniform(const std::string &name, glm::fvec2 val);
    void setUniform(const std::string &name, glm::fvec3 val);
    
    void setUniform(const std::string &name, glm::fmat4 val);
    
    GLint uniformLocation(const std::string &uniformName);
    GLint attribLocation(const std::string &attribName) const;
    
private: 
    GLint getProgramParameter(GLenum pname);
    bool testProgramParameter(GLenum pname, GLint expected = GL_TRUE);
    
    bool m_linked = true;
    GLuint m_programId = 0;
    
    std::unordered_map<std::string, GLint> m_uniformLocations;
    std::unordered_set<std::shared_ptr<Shader>> m_shaders;
};

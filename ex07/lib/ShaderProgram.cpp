#include "ShaderProgram.h"

#include <iostream>

ShaderProgram::ShaderProgram() : m_programId(glCreateProgram())
{
    if(!m_programId || !glIsProgram(m_programId))
    {
        throw std::runtime_error("Create ShaderProgram object failed");
    }
}

ShaderProgram::ShaderProgram(const std::unordered_set<std::shared_ptr<Shader>> &shaders) : ShaderProgram()
{
    m_shaders = shaders;
}

std::unique_ptr<ShaderProgram> ShaderProgram::create()
{
    return std::make_unique<ShaderProgram>();
}

std::unique_ptr<ShaderProgram> ShaderProgram::createFromFiles(const std::string &vertexFileName, const std::string &fragmentFileName)
{
    auto s = ShaderProgram::create();
    
    s->attachShadersFromFile(vertexFileName, fragmentFileName);
    
    if(!s->link()) {
        throw new std::runtime_error("Failed to initialize ShaderProgram");
    }
    
    return s;
}

std::unique_ptr<ShaderProgram> ShaderProgram::create(std::unordered_set<std::shared_ptr<Shader>> shaders)
{
    return std::make_unique<ShaderProgram>(shaders);
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(m_programId);
}

bool ShaderProgram::link()
{
    glLinkProgram(m_programId);
    bool result = testProgramParameter(GL_LINK_STATUS);
    
    if(result)
    {
        m_linked = true;
    }
    else
    {
        std::cout << "Problem linking Shader:";
        std::cout << getDebugLog();
        
        throw std::runtime_error("Link Shader Program failed");
    }
    
    return result;
}

std::string ShaderProgram::getDebugLog()
{
    GLint len = getProgramParameter(GL_INFO_LOG_LENGTH);
    char *msg = new char[len];
    
    glGetProgramInfoLog(m_programId, len, nullptr, msg);
    
    return std::string(msg);
}

bool ShaderProgram::bind()
{
    if((!m_linked && link()) || m_linked)
    {
        glUseProgram(m_programId);
        return true;
    }
    
    return false;
}

void ShaderProgram::unbind()
{
    glUseProgram(0);
}

bool ShaderProgram::testProgramParameter(GLenum pname, GLint expected)
{
    return getProgramParameter(pname) == expected;
}

GLint ShaderProgram::getProgramParameter(GLenum pname)
{
    GLint param = 0;
    glGetProgramiv(m_programId, pname, &param);

    return param;
}


std::shared_ptr<Shader> ShaderProgram::attachShader(Shader::ShaderType type, const std::string &shaderSource, bool fromFile)
{
    std::shared_ptr<Shader> shader;
    
    if(fromFile)
    {
        shader = Shader::createFromFile(type, shaderSource);
    } else
    {
        shader = std::make_shared<Shader>(type, shaderSource);
    }
    
    attachShader(shader);
    
    return shader;
}

void ShaderProgram::attachShadersFromFile(const std::string &vertexFileName, const std::string &fragmentFileName)
{
    attachVertexShader(vertexFileName, true);
    attachFragmentShader(fragmentFileName, true);
}

std::shared_ptr<Shader> ShaderProgram::attachShader(std::shared_ptr<Shader> shader)
{
    m_linked = false;
    
    if(!shader->isCompiled())
    {
        shader->compile();
    }
    
    m_shaders.insert(shader);
    glAttachShader(m_programId, shader->shaderId());
    
    return shader;
}

std::shared_ptr<Shader> ShaderProgram::attachVertexShader(const std::string &shaderSource, bool fromFile)
{
    return attachShader(Shader::ShaderType::Vertex, shaderSource, fromFile);
}

std::shared_ptr<Shader> ShaderProgram::attachFragmentShader(const std::string &shaderSource, bool fromFile)
{
    return attachShader(Shader::ShaderType::Fragment, shaderSource, fromFile);
}

void ShaderProgram::detachShader(std::shared_ptr<Shader> shader)
{
    glDetachShader(m_programId, shader->shaderId());
    m_shaders.erase(shader);
}

void ShaderProgram::setUniform(const std::string &name, GLint val)
{
    int loc = uniformLocation(name);
    
    if(loc != -1) glUniform1i(loc, val);
}

void ShaderProgram::setUniform(const std::string &name, GLfloat val)
{
    int loc = uniformLocation(name);
    
    if(loc != -1) glUniform1f(loc, val);
}

void ShaderProgram::setUniform(const std::string &name, glm::ivec2 val)
{
    int loc = uniformLocation(name);
    
    if(loc != -1) glUniform2i(loc, val.x, val.y);
}

void ShaderProgram::setUniform(const std::string &name, glm::ivec3 val)
{
    int loc = uniformLocation(name);
    
    if(loc != -1) glUniform3i(loc, val.x, val.y, val.z);
}

void ShaderProgram::setUniform(const std::string &name, glm::fvec2 val)
{
    int loc = uniformLocation(name);
    
    if(loc != -1) glUniform2f(loc, val.x, val.y);
}

void ShaderProgram::setUniform(const std::string &name, glm::fvec3 val)
{
    int loc = uniformLocation(name);
    
    if(loc != -1) glUniform3f(loc, val.x, val.y, val.z);
}

void ShaderProgram::setUniform(const std::string &name, glm::fmat4 val)
{
    int loc = uniformLocation(name);
    
    if(loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

GLint ShaderProgram::uniformLocation(const std::string &uniformName)
{
    if(uniformName.empty())
    {
        throw std::runtime_error("uniformName was empty");
    }
    
    if(m_uniformLocations.count(uniformName) > 0)
    {
        return m_uniformLocations[uniformName];
    } else {
        GLint uniformId = glGetUniformLocation(m_programId, uniformName.c_str());
        
        if(uniformId == -1)
        {
            std::cout << "Uniform " << uniformName << " for Shader Program " << m_programId << " not found" << std::endl;
        }
        
        m_uniformLocations[uniformName] = uniformId;
        
        return uniformId;
    }
}

GLint ShaderProgram::attribLocation(const std::string &attribName) const
{
    if(attribName.empty())
    {
        throw std::runtime_error("attribName was empty");
    }
    
    GLint attribId = glGetAttribLocation(m_programId, attribName.c_str());
    
    if(attribId == -1)
    {
        throw std::runtime_error(std::string("Program attribute not found: ") + attribName);
    }
    
    return attribId;
}

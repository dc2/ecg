#include "Shader.h"

#include <fstream>
#include <iostream>

std::string readFile(const std::string &filename)
{
    std::ifstream t(filename);
    std::string str;
    
    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);
    
    str.assign((std::istreambuf_iterator<char>(t)),
                std::istreambuf_iterator<char>());
    
    return str;
}

Shader::Shader(ShaderType type)
{
    m_shaderId = glCreateShader((GLenum)type);
    
    if(!m_shaderId)
    {
        throw std::runtime_error("Create Shader object failed");
    }
}

Shader::Shader(ShaderType type, const std::string &source) : Shader(type)
{
    m_source = source;
    //compile();
}

Shader::~Shader()
{
    glDeleteShader(m_shaderId);
}

std::unique_ptr<Shader> Shader::create(ShaderType type)
{
    return std::make_unique<Shader>(type);
}

std::unique_ptr<Shader> Shader::createFromFile(Shader::ShaderType type, const std::string &filename)
{
    return std::make_unique<Shader>(type, readFile(filename));
}

void Shader::setSource(const std::string &source)
{
    m_source = source;
}

void Shader::compile()
{
    auto source_cstr = m_source.c_str();
    auto source_len = (GLint)m_source.length();
    
    glShaderSource(m_shaderId, 1, &source_cstr, &source_len);
    glCompileShader(m_shaderId);
    
    if(!testShaderParameter(GL_COMPILE_STATUS))
    {
        std::cout  << "Problem compiling Shader:";
        std::cout  << getDebugLog();
        std::cout  << m_source;
        //std::cout  << source << std::endl;
        
        throw std::runtime_error("Shader compilation error");
    }
    
    m_compiled = true;
}

bool Shader::testShaderParameter(GLenum pname, GLint expected)
{
    return getShaderParameter(pname) == expected;
}

std::string Shader::getDebugLog()
{
    GLint len = getShaderParameter(GL_INFO_LOG_LENGTH);
    char *msg = new char[len];
    
    glGetShaderInfoLog(m_shaderId, len, nullptr, msg);
    
    return std::string(msg);
}

GLint Shader::getShaderParameter(GLenum pname)
{
    GLint param;
    glGetShaderiv(m_shaderId, pname, &param);

    return param;
}

#pragma once

#include <string>
#include <memory>
#include <stdexcept>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#if __cplusplus == 201103L

namespace std {
    template<typename T, typename ...Args>
    std::unique_ptr<T> make_unique( Args&& ...args ) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)... ));
    }
}

#endif

class Shader
{
public:
    enum class ShaderType : GLenum {
        Vertex = GL_VERTEX_SHADER,
        Fragment = GL_FRAGMENT_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
        TesselationControl = GL_TESS_CONTROL_SHADER,
        TesselationEvaluation = GL_TESS_EVALUATION_SHADER
    };

    Shader(ShaderType type);
    Shader(ShaderType type, const std::string &source);
    //Shader(const Shader &) = default;
    //Shader(Shader &&) = default;
    ~Shader();
    
    static std::unique_ptr<Shader> create(ShaderType type);
    static std::unique_ptr<Shader> createFromFile(ShaderType type, const std::string &filename);
    
    bool isCompiled() {return m_compiled;}
    
    void compile();
    void setSource(const std::string &source);

    GLuint shaderId() const {return m_shaderId;}
    
private:
    std::string getDebugLog();
    
    GLint getShaderParameter(GLenum pname);
    bool testShaderParameter(GLenum pname, GLint expected = GL_TRUE);
    
    bool m_compiled = false;
    GLuint m_shaderId;
    std::string m_source;
};

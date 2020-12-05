#ifndef UI_SHADER_HPP
#define UI_SHADER_HPP

#include <GL/glew.h>
#include <string>

#include <string>
#include <fstream>
#include <sstream>

namespace astro
{
  static std::string SHADER_LOG = ""; // running log of shader compilation
  
  static GLuint loadShader(GLenum type, const std::string &path)
  {
    std::string typeStr = "";
    switch(type)
      {
      case GL_VERTEX_SHADER:
        typeStr = "Vertex"; break;
      case GL_FRAGMENT_SHADER:
        typeStr = "Fragment"; break;
      case GL_GEOMETRY_SHADER:
        typeStr = "Geometry"; break;
      case GL_COMPUTE_SHADER:
        typeStr = "Compute"; break;
      }
    std::cout << "Opening " << typeStr << " shader -->  '" << path << "'\n";
    std::ifstream file(path, std::ios::in);
    std::stringstream src; src << file.rdbuf();
    std::string shaderStr = src.str();
    const char* shaderCode = shaderStr.c_str();
  
    int success;
    GLuint shaderId = glCreateShader(type);
    glShaderSource(shaderId, 1, &shaderCode, NULL);
    glCompileShader(shaderId);
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if(!success)
      {
        int maxLength;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);
        char *infoLog = new char[maxLength];
        glGetShaderInfoLog(shaderId, maxLength, NULL, infoLog);
      
        SHADER_LOG += "--- " + typeStr + " Shader Log:\n" + "---\n  | " + infoLog + "\n---=---\n";

        shaderId = 0;
        delete infoLog;
      }
    return shaderId;
  }

  static bool linkProgram(GLuint shader)
  {
    glLinkProgram(shader);
    int success;
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if(!success)
      {
        int maxLength;
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        char *infoLog = new char[maxLength];
        glGetProgramInfoLog(shader, maxLength, NULL, infoLog);
        SHADER_LOG += std::string("--- Shader Program Link Log:\n---\n  | ") + infoLog + "\n---=---\n";
        
        shader = 0;
        delete infoLog;
        return false;
      }
    else
      { return true; }
  }
};

#endif // UI_SHADER_HPP

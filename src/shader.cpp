#include <iostream>
#include <QMessageBox>

#include "shader.hpp"


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
Shader::Shader():
  QOpenGLFunctions(QOpenGLContext::currentContext())
{

}
//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
Shader::~Shader()
{

}

//-----------------------------------------------------------------------------
// enable/disable
//-----------------------------------------------------------------------------
void Shader::enable()
{
  this->glUseProgram(program);
}
void Shader::disable()
{
  this->glUseProgram(0);
}


//-----------------------------------------------------------------------------
// send uniform
//-----------------------------------------------------------------------------
void Shader::sendi(const char *name, int x)
{
	glUniform1i(glGetUniformLocation(program,name), x);
}
void Shader::sendf(const char *name, float x)
{
	glUniform1f(glGetUniformLocation(program,name), x);
}
void Shader::sendf(const char *name, float x, float y)
{
    glUniform2f(glGetUniformLocation(program,name), x,y);
}
void Shader::sendf(const char *name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(program,name), x,y,z);
}
void Shader::sendf(const char *name, float x, float y, float z, float w)
{
	glUniform4f(glGetUniformLocation(program,name), x,y,z,w);
}


//-----------------------------------------------------------------------------
// compil
//-----------------------------------------------------------------------------
int Shader::compil( const char *vertex, const char *fragment )
{
  program = this->glCreateProgram();
	if( !makeShader(vertex, GL_VERTEX_SHADER_ARB) )
    {
        //QMessageBox::critical(NULL, "Error", "Can't compil the vertex shader !");
		return SHADER_VERTEX_ERROR;
    }
    if( !makeShader(fragment, GL_FRAGMENT_SHADER_ARB) )
    {
        //QMessageBox::critical(NULL, "Error", "Can't compil this pixel shader !");
        return SHADER_FRAGMENT_ERROR;
    }

    glLinkProgram(program);

	return SHADER_SUCCESS;
}


bool Shader::makeShader(const char *txt, GLuint type)
{
    GLuint object = glCreateShader(type);
	glShaderSource(object, 1, (const GLchar**)(&txt), NULL);
	glCompileShader(object);
	
    if( checkShader(object) )
    {
      this->glAttachShader(program, object);
      this->glDeleteShader(object);
    }
    else
    {
        return false;
    }
    return true;
}

bool Shader::checkShader(int id)
{
    GLint ok = 0;

    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if( ok != GL_TRUE )
    {
        GLint logLength = 0;
        GLchar* log = NULL;

        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
        log = new GLchar[logLength];
        glGetShaderInfoLog(id, logLength, &logLength, log);

        emit error(QString(log));
        delete[] log;
        return false;
    }
    else
    {
        GLint logLength = 0;
        GLchar* log = NULL;

        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
        log = new GLchar[logLength];
        glGetShaderInfoLog(id, logLength, &logLength, log);

        if(logLength>0)
        {
          emit warning(QString(log));
        }
        delete[] log;
    }

    return true;
}

const char* Shader::getVertexShader()
{

  static const char* vertexShader =
  "#version 120\n"
  "varying vec2 coords;"
  "void main()"
  "{"
     "coords         = gl_Vertex.xy;"
     "gl_Position    = gl_Vertex;"
     "gl_TexCoord[0] = gl_MultiTexCoord0;"
  "}";
  return vertexShader;

}




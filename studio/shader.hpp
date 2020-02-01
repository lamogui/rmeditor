#ifndef SHADER_HPP
#define SHADER_HPP

#include <QObject>
#include <QString>
#include <QOpenGLFunctions>

#define SHADER_SUCCESS        1
#define SHADER_VERTEX_ERROR   2
#define SHADER_FRAGMENT_ERROR 3

class Shader : public QObject, protected QOpenGLFunctions
{
  Q_OBJECT

public:
	Shader();
  virtual ~Shader();
	
	int compil(const char *vertex, const char *fragment);

	void enable();
	void disable();
	
  void sendi(const char *name, int x);
  void sendf(const char *name, float x);
  void sendf(const char *name, float x, float y);
  void sendf(const char *name, float x, float y, float z);
  void sendf(const char *name, float x, float y, float z, float w);

  static const char* getVertexShader();

private:
  bool makeShader(const char *txt, GLuint type);
  bool checkShader( int id );
  GLuint m_program;
};



#endif

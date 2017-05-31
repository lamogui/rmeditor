#ifndef SCENE_HPP
#define SCENE_HPP

#include "shadercode.hpp"

class Camera;
class Framework;
class Scene : public FragmentShaderCode
{
public:
  Scene(const QString& filename, QDomNode node, Framework* framework, LogWidget& log,QObject* parent);

  Renderer* getRenderer() const override;
  
  QSharedPointer<QOpenGLShaderProgram>& getShaderProgram() {return shaderProgram;}
  const QSharedPointer<QOpenGLShaderProgram>& getShaderProgram() const {return shaderProgram; }

  bool buildable() const override { return true; }

  inline Framework* getFramework() const { return framework; }

  static const char* getVertexShaderCode();


public slots:
  bool build(const QString& text) override;

private:
  Framework* framework;
  QSharedPointer<Camera> camera;
  QSharedPointer<ShaderProgram> shaderProgram;

private:
  typedef FragmentShaderCode BaseClass;
};

#endif

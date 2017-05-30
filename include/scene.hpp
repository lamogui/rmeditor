#ifndef SCENE_HPP
#define SCENE_HPP

#include "camera.hpp"
#include "shadercode.hpp"

class Framework;
class SceneRender;


class Scene : public FragmentShaderCode
{
public:
  Scene(const QString& filename, QDomNode node, Framework* framework, LogWidget& log,QObject* parent);

  Render* getRender() const override;
  
  QSharedPointer<QOpenGLShaderProgram>& getShaderProgram() {return shaderProgram;}
  const QSharedPointer<QOpenGLShaderProgram>& getShaderProgram() const {return shaderProgram; }

  inline Framework* getFramework() const { return framework; }

  static const char* getVertexShaderCode();


public slots:
  bool build(const QString& text) override;

private:
  Framework* framework;
  QSharedPointer<Camera> camera;
  QSharedPointer<QOpenGLShaderProgram> shaderProgram;

private:
  typedef FragmentShaderCode BaseClass;
};

#endif

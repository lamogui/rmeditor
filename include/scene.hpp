#ifndef SCENE_HPP
#define SCENE_HPP

#include "camera.hpp"
#include "shadercode.hpp"

class Framework;
class SceneRenderer;


class Scene : public FragmentShaderCode
{
public:
  Scene(const QString& filename, QDomNode node, Framework* framework, LogWidget& log,QObject* parent);
  virtual ~Scene();

  virtual Renderer* getRenderer() const;

  inline Framework* framework() const { return m_framework; }

public slots:
  virtual bool build(const QString& text);

private:

  Framework* m_framework;
  SceneRenderer* m_sceneRenderer;
  Camera m_camera;

};

#endif

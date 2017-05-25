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
  ~Scene() override;

  Renderer* getRenderer() const override;

  inline Framework* getFramework() const { return framework; }

public slots:
  bool build(const QString& text) override;

private:

  Framework* framework;
  SceneRenderer* sceneRenderer;
  Camera camera;

};

#endif

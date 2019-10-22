
#ifndef FRAMEWORK_HPP
#define FRAMEWORK_HPP

#include "shadercode.hpp"

class Framework : public FragmentShaderCode
{  
  public:
    Framework(const QString& filename, QDomNode node, LogWidget& log,QObject* parent):
      FragmentShaderCode(filename,node,log,parent)
    {
      load();
    }

    inline virtual bool buildable() const { return false; }

public slots:
  virtual bool build(const QString& text)
  {
    m_fragmentcode = text;
    return true;
  }

  private:

};


#endif

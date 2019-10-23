
#ifndef FRAMEWORK_HPP
#define FRAMEWORK_HPP

#include "shadercode.hpp"

class Framework : public FragmentShaderCode
{  
  public:
		Framework(const QString& _filename, QDomNode _node, QObject* _parent):
			FragmentShaderCode(_filename, _node, _parent)
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

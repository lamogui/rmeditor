

#ifndef TEXTEDITABLE_HPP
#define TEXTEDITABLE_HPP

#include "nodefile.hpp"

class LogWidget;
class Renderer;

class TextEditable : public NodeFile
{

  Q_OBJECT

public:
  TextEditable(const QString& filename, QDomNode node ,LogWidget& log,QObject* parent);

  //The text to print in the editor and that will be saved in the file
  virtual const QString& getText() const = 0;


  //Is the object buildable
  virtual bool buildable() const {return false;}
  inline virtual Renderer* getRenderer() const {return nullptr;}

public slots:
  //Return true is the file is loaded or created (not if it build correctly)
  bool load();
  virtual bool save();
  virtual bool build(const QString& text) = 0;


signals:
  void startLineNumberChanged(int line);
  void objectTextChanged(QString text);


protected:
  bool save(const QString& text);

};

#endif

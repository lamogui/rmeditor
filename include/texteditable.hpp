

#ifndef TEXTEDITABLE_HPP
#define TEXTEDITABLE_HPP

#include "mediafile.hpp"

class Renderer;
class TextEditable : public MediaFile /* Represent any text compilable text media */
{
  Q_OBJECT

public:
  TextEditable();

  //The text to print in the editor and that will be saved in the file
  virtual const QString& getText() const = 0;

  //Is the text object buildable
  virtual bool buildable() const {return false;}

public slots:
  //Return true is the file is loaded or created (not if it build correctly)
  bool load();
  virtual bool save();
  virtual bool build(const QString& text) { return false; }


signals:
  // text edition 
  void startLineNumberChanged(int line);
  void objectTextChanged(QString text); // for project (probably)

  // Log
  void error(QString);
  void warning(QString);

protected:
  bool save(const QString& text);
};

#endif

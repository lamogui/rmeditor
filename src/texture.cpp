﻿#include <QImage>
#include <QGLWidget>
#include <QMessageBox>
#include <iostream>
#include <QTextStream>
#include <QFile>
#include "texture.hpp"


AbstractTexture::AbstractTexture():
  QOpenGLFunctions(QOpenGLContext::currentContext()),
  m_id(-1)
{
     this->initializeOpenGLFunctions();
}

AbstractTexture::~AbstractTexture()
{
  destroyTexture();
}

void AbstractTexture::destroyTexture()
{
  if (m_id == -1)
  {
    glDeleteTextures(1 , &m_id);
    m_id = -1;
  }
}

Texture2D::Texture2D()
{
}

void Texture2D::bind()
{
    glBindTexture(GL_TEXTURE_2D, m_id);
}

bool Texture2D::create(GLsizei width, GLsizei height, const GLvoid *data, GLint internalFormat, GLenum format, GLenum type)
{
  destroyTexture();

  m_width = width;
  m_height = height;
  m_format = format;
  m_type = type;
  glGenTextures( 1, &m_id );
  glBindTexture( GL_TEXTURE_2D, m_id );
  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
  return true; //TODO check errors
}

bool Texture2D::load(const std::string &path)
{

    QImage t;
    QImage b;

    if( !b.load( path.c_str() ) )
    {
        QMessageBox::critical(NULL, "Error", QString(QString("Can't load image ")+path.c_str()) );
        return false;
    }
    t = QGLWidget::convertToGLFormat( b );
    return create(t.width(), t.height(),t.bits(),GL_RGBA,GL_RGBA,GL_UNSIGNED_BYTE);
}

void Texture2D::update(GLvoid *pixels)
{
  if (m_id != -1)
  {
    bind();
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,this->width(),this->height(),m_format,m_type,pixels);
  }
}

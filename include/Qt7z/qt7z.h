/****************************************************************************
 *
 * Copyright (C) 2015 Neutrino International Inc.
 *
 * Author : Brian Lin <lin.foxman@gmail.com>, Skype: wolfram_lin
 *
 * QtCURL acts as an interface between Qt and cURL library.
 * Please keep QtCURL as simple as possible.
 *
 ****************************************************************************/

#ifndef QT_7Z_H
#define QT_7Z_H

#include <QtCore>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#  if defined(QT_BUILD_QT7Z_LIB)
#    define Q_7Z_EXPORT Q_DECL_EXPORT
#  else
#    define Q_7Z_EXPORT Q_DECL_IMPORT
#  endif
#else
#    define Q_7Z_EXPORT
#endif

class Q_7Z_EXPORT Qt7z
{
  public:

    QMap<QString,QVariant> Variables ;

    explicit     Qt7z          (void) ;
    virtual     ~Qt7z          (void) ;

    virtual bool extract       (QString filename) ;
    virtual bool extract       (QString filename,QDir & directory) ;

  protected:

    virtual void report        (QString message) ;
    virtual void error         (int code,QString message) ;

    virtual void setAttributes (QString         filename     ,
                                unsigned int    attributes ) ;
    virtual void create        (QDir          & directory    ,
                                QString         name       ) ;
    virtual bool write         (QString         filename     ,
                                unsigned char * buffer       ,
                                qint64        & size       ) ;
    virtual bool write         (QDir          & directory    ,
                                QString         name         ,
                                unsigned char * buffer       ,
                                qint64        & size       ) ;

  private:

};

Q_DECLARE_METATYPE(Qt7z)

QT_END_NAMESPACE

#endif

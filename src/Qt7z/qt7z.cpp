/****************************************************************************
 *
 * Copyright (C) 2001 ~ 2016 Neutrino International Inc.
 *
 ****************************************************************************/

#include <qt7z.h>

#include "7zVersion.h"
#include "7z.h"
#include "7zAlloc.h"
#include "7zBuf.h"
#include "7zCrc.h"
#include "7zFile.h"
#include "Bcj2.h"
#include "Bra.h"
#include "CpuArch.h"
#include "Lzma2Dec.h"
#include "LzmaDec.h"
#include "Ppmd.h"
#include "Ppmd7.h"
#include "7zTypes.h"

QT_BEGIN_NAMESPACE

Qt7z:: Qt7z(void)
{
}

Qt7z::~Qt7z(void)
{
}

void Qt7z::report(QString message)
{
}

void Qt7z::error(int code,QString message)
{
  Variables [ "ErrorCode"    ] = code    ;
  Variables [ "ErrorMessage" ] = message ;
}

void Qt7z::setAttributes(QString filename,unsigned int attributes)
{
  #ifdef Q_OS_WIN
  ::SetFileAttributesW ( (LPCWSTR) filename . utf16 ( ) , attributes ) ;
  #endif
}

void Qt7z::create(QDir & d,QString name)
{
  QString n = d . absoluteFilePath ( name ) ;
  d . mkpath ( n )                          ;
}

bool Qt7z::extract(QString filename)
{
  QDir dir = QDir::current ( )      ;
  return extract ( filename , dir ) ;
}

bool Qt7z::write                (
       QString         filename ,
       unsigned char * buffer   ,
       qint64        & size     )
{
  QFile F ( filename )                                    ;
  if ( ! F . open ( QIODevice::WriteOnly ) )              {
    error ( SZ_ERROR_FAIL , "can not open output file" )  ;
    return false                                          ;
  }                                                       ;
  qint64 w                                                ;
  bool   correct = true                                   ;
  w = F . write ( (const char *) buffer , size )          ;
  if ( w != size )                                        {
    error ( SZ_ERROR_FAIL , "can not write output file" ) ;
    correct = false                                       ;
  }                                                       ;
  size = w                                                ;
  F . close ( )                                           ;
  return correct                                          ;
}

bool Qt7z::write                 (
       QDir          & directory ,
       QString         name      ,
       unsigned char * buffer    ,
       qint64        & size      )
{
  QString filename = directory . absoluteFilePath ( name ) ;
  return write ( filename , buffer , size )                ;
}

bool Qt7z::extract(QString filename,QDir & directory)
{
  QFileInfo FI ( filename )                                                  ;
  if ( ! FI . exists ( ) ) return false                                      ;
  ////////////////////////////////////////////////////////////////////////////
  QByteArray    FILENAME = filename . toUtf8 ( )                             ;
  CFileInStream archiveStream                                                ;
  CLookToRead   lookStream                                                   ;
  CSzArEx       db                                                           ;
  ISzAlloc      allocImp                                                     ;
  ISzAlloc      allocTempImp                                                 ;
  SRes          res                                                          ;
  UInt16      * temp     = NULL                                              ;
  size_t        tempSize = 0                                                 ;
  ////////////////////////////////////////////////////////////////////////////
  allocImp     . Alloc = ::SzAlloc                                           ;
  allocImp     . Free  = ::SzFree                                            ;
  allocTempImp . Alloc = ::SzAllocTemp                                       ;
  allocTempImp . Free  = ::SzFreeTemp                                        ;
  ////////////////////////////////////////////////////////////////////////////
  if ( 0 != InFile_Open ( & archiveStream . file , FILENAME.constData() ) )  {
    error ( 101 , QString("Can not open %1").arg(filename) )                 ;
    return false                                                             ;
  }                                                                          ;
  ::FileInStream_CreateVTable ( & archiveStream      )                       ;
  ::LookToRead_CreateVTable   ( & lookStream , False )                       ;
  ////////////////////////////////////////////////////////////////////////////
  lookStream . realStream = &archiveStream . s                               ;
  ::LookToRead_Init           ( & lookStream         )                       ;
  ::CrcGenerateTable          (                      )                       ;
  ::SzArEx_Init               ( & db                 )                       ;
  res = ::SzArEx_Open ( &db , &lookStream.s , &allocImp , &allocTempImp )    ;
  ////////////////////////////////////////////////////////////////////////////
#ifdef THIS_IS_OLD_7Z_CODE
  if ( SZ_OK == res )                                                        {
    UInt32 i                                                                 ;
    UInt32 blockIndex    = 0xFFFFFFFF                                        ;
    Byte * outBuffer     = 0                                                 ;
    size_t outBufferSize = 0                                                 ;
    //////////////////////////////////////////////////////////////////////////
    for ( i = 0 ; i < db . db . NumFolders ; i++ )                           {
      size_t              offset           = 0                               ;
      size_t              outSizeProcessed = 0                               ;
      const CSzFileItem * f                = db . db . Files + i             ;
      size_t              len                                                ;
      ////////////////////////////////////////////////////////////////////////
      len = ::SzArEx_GetFileNameUtf16 ( &db , i , NULL )                     ;
      if ( len > tempSize )                                                  {
        ::SzFree ( NULL , temp )                                             ;
        tempSize = len                                                       ;
        temp     = (UInt16 *)::SzAlloc(NULL, tempSize * sizeof(temp[0]))     ;
        if ( temp == 0 )                                                     {
          res = SZ_ERROR_MEM                                                 ;
          break                                                              ;
        }                                                                    ;
      }                                                                      ;
      ////////////////////////////////////////////////////////////////////////
      ::SzArEx_GetFileNameUtf16 ( &db , i , temp )                           ;
      if ( f -> IsDir )                                                      {
        report ( "/" )                                                       ;
      } else                                                                 {
        res = ::SzArEx_Extract                                               (
                &db                                                          ,
                &lookStream.s                                                ,
                 i                                                           ,
                &blockIndex                                                  ,
                &outBuffer                                                   ,
                &outBufferSize                                               ,
                &offset                                                      ,
                &outSizeProcessed                                            ,
                &allocImp                                                    ,
                &allocTempImp                                              ) ;
        if ( SZ_OK != res ) break                                            ;
      }                                                                      ;
      ////////////////////////////////////////////////////////////////////////
      qint64         processedSize                                           ;
      size_t         j                                                       ;
      UInt16       * name     = (UInt16       *) temp                        ;
      const UInt16 * destPath = (const UInt16 *) name                        ;
      QString        destFile = QString::fromUtf16 ( destPath )              ;
      ////////////////////////////////////////////////////////////////////////
      for ( j = 0 ; name [ j ] != 0 ; j++ ) if ( name [ j ] == '/' )         {
        QString pname                                                        ;
        name [ j ] = 0                                                       ;
        pname = QString::fromUtf16 ( name )                                  ;
        create ( directory , pname )                                         ;
        name [ j ] = CHAR_PATH_SEPARATOR                                     ;
      }                                                                      ;
      ////////////////////////////////////////////////////////////////////////
      if ( f -> IsDir )                                                      {
        create ( directory , destFile )                                      ;
      } else                                                                 {
        processedSize = outSizeProcessed                                     ;
        if ( ! write ( directory                                             ,
                       destFile                                              ,
                       outBuffer + offset                                    ,
                       processedSize                                     ) ) {
          return false                                                       ;
        }                                                                    ;
      }                                                                      ;
      ////////////////////////////////////////////////////////////////////////
      if ( f -> AttribDefined )                                              {
        QString fname = directory . absoluteFilePath ( destFile )            ;
        setAttributes ( fname , f -> Attrib )                                ;
      }                                                                      ;
    }                                                                        ;
    IAlloc_Free ( &allocImp , outBuffer )                                    ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  ::SzArEx_Free ( &db  , & allocImp     )                                    ;
  ::SzFree      ( NULL ,   temp         )                                    ;
  ::File_Close  ( &archiveStream . file )                                    ;
#endif
  if ( SZ_OK == res ) return true                                            ;
  ////////////////////////////////////////////////////////////////////////////
  if ( SZ_ERROR_UNSUPPORTED == res )                                         {
    error ( SZ_ERROR_UNSUPPORTED , "decoder doesn't support this archive" )  ;
  } else
  if ( SZ_ERROR_MEM == res )                                                 {
    error ( SZ_ERROR_MEM , "can not allocate memory" )                       ;
  } else
  if ( SZ_ERROR_CRC == res )                                                 {
    error ( SZ_ERROR_CRC , "CRC error" )                                     ;
  } else                                                                     {
    error ( 999 , QString("\nERROR #%1").arg(res) )                          ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  return false                                                               ;
}

QT_END_NAMESPACE

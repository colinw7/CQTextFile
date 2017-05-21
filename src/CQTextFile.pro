TEMPLATE = lib

QT += widgets

TARGET = CQTextFile

DEPENDPATH += .

QMAKE_CXXFLAGS += -std=c++11

CONFIG += staticlib

MOC_DIR = .moc

# Input
SOURCES += \
CQTextFile.cpp \
CTextFileBuffer.cpp \
CTextFile.cpp \
CTextFileEd.cpp \
CTextFileKey.cpp \
CTextFileMarks.cpp \
CTextFileNormalKey.cpp \
CTextFileSel.cpp \
CTextFileUndo.cpp \
CTextFileUtil.cpp \
CTextFileViKey.cpp \

HEADERS += \
../include/CQTextFileCanvas.h \
../include/CQTextFile.h \
../include/CTextFileBuffer.h \
../include/CTextFileEd.h \
../include/CTextFile.h \
../include/CTextFileKey.h \
../include/CTextFileMarks.h \
../include/CTextFileNormalKey.h \
../include/CTextFileSel.h \
../include/CTextFileUndo.h \
../include/CTextFileUtil.h \
../include/CTextFileViKey.h \

DESTDIR     = ../lib
OBJECTS_DIR = ../obj

INCLUDEPATH += \
. \
../include \
../../CQUtil/include \
../../CCommand/include \
../../CImageLib/include \
../../CUndo/include \
../../CFont/include \
../../CFileUtil/include \
../../CFile/include \
../../CConfig/include \
../../COS/include \
../../CStrUtil/include \
../../CUtil/include \
../../CMath/include \
../../CReadLine/include \
../../CRegExp/include \
../../CRGBName/include \

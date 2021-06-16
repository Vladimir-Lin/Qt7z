NAME         = Qt7z
TARGET       = $${NAME}
QT           = core
QT          -= gui
CONFIG(static,static|shared) {
# static version does not support Qt Script now
QT          -= script
} else {
# QT          += script
}

load(qt_module)

INCLUDEPATH += $${PWD}/../../include/Qt7z

HEADERS     += $${PWD}/../../include/Qt7z/qt7z.h

include ($${PWD}/7z/7z.pri)

SOURCES     += qt7z.cpp

OTHER_FILES += $${PWD}/../../include/$${NAME}/headers.pri

include ($${PWD}/../../doc/Qt/Qt.pri)

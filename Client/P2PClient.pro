#-------------------------------------------------
#
# Project created by QtCreator 2017-10-23T14:27:03
#
#-------------------------------------------------

QT       += core gui
QT += network
greaterThan(QT_MAJOR_VERSION, 4):
QT += widgets

TARGET = P2PClient
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        clientwindow.cpp \
    Kuznyechik.cpp \
    mycrypto.cpp    \
    peer.cpp \
    issuecreator.cpp


HEADERS += \
        clientwindow.h \
    Kuznyechik.hpp \
    mycrypto.hpp    \
    peer.h \
    issuecreator.h



FORMS += \
        clientwindow.ui \
    issuecreator.ui


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/cryptopp/release/ -lcryptopp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/cryptopp/debug/ -lcryptopp
else:unix: LIBS += -L$$PWD/cryptopp/ -lcryptopp

INCLUDEPATH += $$PWD/cryptopp
DEPENDPATH += $$PWD/cryptopp

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/cryptopp/release/libcryptopp.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/cryptopp/debug/libcryptopp.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/cryptopp/release/cryptopp.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/cryptopp/debug/cryptopp.lib
else:unix: PRE_TARGETDEPS += $$PWD/cryptopp/libcryptopp.a

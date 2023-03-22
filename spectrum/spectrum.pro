#-------------------------------------------------
#
# Project created by QtCreator 2020-02-15T20:41:09
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = spectrum
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
        mainwindow.cpp \
    audio.cpp \
    kiss_fft.c \
    spectrumcalculator.cpp \
    spectraldata.cpp \
    renderwaveform.cpp \
    renderer.cpp \
    renderspectrum.cpp \
    renderdebug.cpp

HEADERS += \
        mainwindow.h \
    audio.h \
    _kiss_fft_guts.h \
    kiss_fft.h \
    spectrumcalculator.h \
    spectraldata.h \
    renderwaveform.h \
    renderer.h \
    renderspectrum.h \
    renderdebug.h

FORMS += \
        mainwindow.ui

#-------------------------------------------------
#
# Project created by QtCreator 2017-07-31T14:49:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = opencv_test
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h

FORMS    += widget.ui

INCLUDEPATH   += /usr/local/include/opencv \
                 /usr/local/include  \
                 /udt/local/include/opencv2


LIBS      +=   -L/usr/local/lib  \
               -lopencv_shape \
               -lopencv_stitching  \
               -lopencv_objdetect \
               -lopencv_superres \
               -lopencv_videostab \
               -lopencv_calib3d \
               -lopencv_features2d \
               -lopencv_highgui \
               -lopencv_videoio \
               -lopencv_imgcodecs \
               -lopencv_video \
               -lopencv_photo \
               -lopencv_ml \
               -lopencv_imgproc \
               -lopencv_flann \
               -lopencv_core \

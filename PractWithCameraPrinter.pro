QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cameraconnect.cpp \
    main.cpp \
    mainwindow.cpp


INCLUDEPATH += "C:/Program Files/Basler/pylon 6/Development/include"

LIBS += "-LC:/Program Files/Basler/pylon 6/Development/lib/x64" \
    -lGenApi_MD_VC141_v3_1_Basler_pylon \
    -lGCBase_MD_VC141_v3_1_Basler_pylon\
    -lPylonBase_v6_2 \

INCLUDEPATH += "C:/Qt/opencv/include"
LIBS += -LC:\Qt\opencv\lib\Release\
            -lopencv_aruco430 \
            -lopencv_bgsegm430 \
            -lopencv_bioinspired430 \
            -lopencv_calib3d430 \
            -lopencv_ccalib430 \
            -lopencv_core430 \
            -lopencv_cvv430 \
            -lopencv_datasets430 \
            -lopencv_dnn430 \
            -lopencv_dnn_objdetect430 \
            -lopencv_dnn_superres430 \
            -lopencv_dpm430 \
            -lopencv_face430 \
            -lopencv_features2d430 \
            -lopencv_flann430 \
            -lopencv_fuzzy430 \
            -lopencv_gapi430 \
            -lopencv_hfs430 \
            -lopencv_highgui430 \
            -lopencv_imgcodecs430 \
            -lopencv_imgproc430 \
            -lopencv_img_hash430 \
            -lopencv_intensity_transform430 \
            -lopencv_line_descriptor430 \
            -lopencv_ml430 \
            -lopencv_objdetect430 \
            -lopencv_optflow430 \
            -lopencv_phase_unwrapping430 \
            -lopencv_photo430 \
            -lopencv_plot430 \
            -lopencv_quality430 \
            -lopencv_rapid430 \
            -lopencv_reg430 \
            -lopencv_rgbd430 \
            -lopencv_saliency430 \
            -lopencv_shape430 \
            -lopencv_stereo430 \
            -lopencv_stitching430 \
            -lopencv_structured_light430 \
            -lopencv_superres430 \
            -lopencv_surface_matching430 \
            -lopencv_text430 \
            -lopencv_tracking430 \
            -lopencv_ts430 \
            -lopencv_video430 \
            -lopencv_videoio430 \
            -lopencv_videostab430 \
            -lopencv_xfeatures2d430 \
            -lopencv_ximgproc430 \
            -lopencv_xobjdetect430 \
            -lopencv_xphoto430

HEADERS += \
    cameraconnect.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

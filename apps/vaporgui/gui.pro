# ------------------------------------------- 
# Subdir relative project main directory: ./src/apps/gui
# Target is an application:  ../../platform/bin/vaporgui
# This is not used for building Makefiles, only used for
# building vcproj.  All relative paths for linking files are
# relative to the location of the vcproj file, which is 
# located at ../../make/win32/vcproj/

TEMPLATE = app

win32:TARGET = ../../../targets/win32/bin/vaporgui

win32:OBJECTS_DIR = ../../../targets/win32/built/vaporgui/


win32:LIBS += ../../../targets/win32/bin/vdf.lib
win32:LIBS += ../../../targets/win32/bin/common.lib

CONFIG += opengl 
CONFIG += debug
	
CONFIG -= dlopen_opengl
REQUIRES = opengl

win32:CONFIG += thread

win32:LIBS += $(VOLUMIZER_ROOT)/lib/vz.lib
win32:INCLUDEPATH += $(VOLUMIZER_ROOT)/include
win32:INCLUDEPATH += "C:/Expat-1.95.8/Source/lib"
win32:QMAKE_CXXFLAGS_DEBUG += /EHsc
win32:QMAKE_CXXFLAGS_RELEASE += /EHsc
DEFINES += VOLUMIZER


INCLUDEPATH += . \
	../../include \
	../../../apps/vaporgui/misc \
	../../../apps/vaporgui/guis \
	../../../apps/vaporgui/main \
	../../../apps/vaporgui/render

MOC_DIR = ../../../apps/vaporgui/moc

SOURCES +=\ 
	   misc/animationcontroller.cpp \
	   guis/coloradjustdialog.cpp \
	   guis/colorpicker.cpp \
	   main/command.cpp \
	   render/DVRBase.cpp \
	   render/DVRDebug.cpp \
	   render/DVRVolumizer.cpp \
	   guis/flowmapframe.cpp \
	   render/flowrenderer.cpp \
	   render/glwindow.cpp \
 	   guis/loadtfdialog.cpp \
	   main/main.cpp \
           main/MainForm.cpp \
	   main/MessageReporter.cpp \	
	   guis/opacadjustdialog.cpp \
	   main/panelcommand.cpp \
	   render/Renderer.cpp \
	   guis/savetfdialog.cpp \
	   main/session.cpp \
	   misc/sessionparams.cpp \
	   misc/sharedcontrollerthread.cpp \
           main/TabManager.cpp \
	   guis/tfelocationtip.cpp \
	   guis/tfframe.cpp \
	   render/TrackBall.cpp \
	   misc/unsharedcontrollerthread.cpp \
	   misc/VizFeatureParams.cpp \
	   main/vizactivatecommand.cpp \
	   guis/VizSelectCombo.cpp \
           render/VizWin.cpp \
           main/VizWinMgr.cpp \
	   render/volumizerrenderer.cpp 
	   
HEADERS += \
	   misc/animationcontroller.h \
	   guis/coloradjustdialog.h \
	   guis/colorpicker.h \
	   main/command.h \
	   misc/controllerthread.h \
	   guis/flowmapframe.h \
	   render/DVRBase.h \
	   render/DVRDebug.h \
	   render/DVRVolumizer.h \
	   render/flowrenderer.h \
	   render/glbox.h \
	   render/glwindow.h \
	   render/glutil.h \
	   misc/VizFeatureParams.h \
	   guis/loadtfdialog.h \
           main/MainForm.h \
	   main/MessageReporter.h \
	   guis/opacadjustdialog.h \
	   main/panelcommand.h \
	   render/Renderer.h \
	   guis/savetfdialog.h \
	   main/session.h \
	   misc/sessionparams.h \
           main/TabManager.h \
           guis/tfelocationtip.h \
	   guis/tfframe.h \
	   render/TrackBall.h \
	   main/vizactivatecommand.h \
	   guis/VizSelectCombo.h \
	   render/VizWin.h \
           main/VizWinMgr.h \
	   render/volumizerrenderer.h 

FORMS +=  ../../../guis/ui/animationtab.ui \
	 ../../../guis/ui/viztab.ui \
	 ../../../guis/ui/dvr.ui \
	 ../../../guis/ui/flowtab.ui \
         ../../../guis/ui/regiontab.ui \ 
	 ../../../guis/ui/sessionparameters.ui \ 
	 ../../../guis/ui/vizfeatures.ui



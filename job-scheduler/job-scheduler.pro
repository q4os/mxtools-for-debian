QT       += core gui widgets
CONFIG   += c++17

TEMPLATE = app
TARGET = job-scheduler

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += Clib.h \
           CronModel.h \
           Crontab.h \
           CronTime.h \
           CronView.h \
           Execute.h \
           ExecuteList.h \
           ExecuteModel.h \
           ExecuteView.h \
           MainWindow.h \
           SaveDialog.h \
           TCommandEdit.h \
           TimeDialog.h \
           VariableEdit.h \
           VariableModel.h \
           VariableView.h \
           Version.h \
           about.h
SOURCES += CronModel.cpp \
           Crontab.cpp \
           CronTime.cpp \
           CronView.cpp \
           ExecuteList.cpp \
           ExecuteModel.cpp \
           ExecuteView.cpp \
           about.cpp \
           main.cpp \
           MainWindow.cpp \
           SaveDialog.cpp \
           TCommandEdit.cpp \
           TimeDialog.cpp \
           VariableEdit.cpp \
           VariableModel.cpp \
           VariableView.cpp
RESOURCES += application.qrc

TRANSLATIONS += translations/job-scheduler_en.ts

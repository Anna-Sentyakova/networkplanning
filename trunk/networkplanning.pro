HEADERS = treeitem.h \
    treemodel.h \
    mainwindow.h \
    ui_mainwindow.h \
    netmodel.h \
    operationdelegate.h \
    eventwidget.h \
    positioning.h \
    graphwidget.h \
    arrowwidget.h
RESOURCES = 
SOURCES = treeitem.cpp \
    treemodel.cpp \
    main.cpp \
    mainwindow.cpp \
    netmodel.cpp \
    operationdelegate.cpp \
    positioning.cpp \
    graphwidget.cpp \
    arrowwidget.cpp
CONFIG += qt

# install
target.path = $$[QT_INSTALL_EXAMPLES]/itemviews/simpletreemodel
sources.files = $$SOURCES \
    $$HEADERS \
    $$RESOURCES \
    *.pro \
    *.txt
sources.path = $$[QT_INSTALL_EXAMPLES]/itemviews/simpletreemodel
INSTALLS += target \
    sources
FORMS += mainwindow.ui

DEFINES += QTCUTILITIES_LIBRARY

include(paths.pri)

# QtcUtilities files

SOURCES += \
    src/QtcUtilitiesPlugin.cpp \
    src/codediscover/CodeDiscover.cpp \
    src/codediscover/CodeDiscoverOptionsPage.cpp \
    src/codediscover/CodeDiscoverWindow.cpp \
    src/codediscover/CodeDiscoverToolRunner.cpp \
    src/codediscover/ClassDiagramGenerator.cpp \
    src/ci/Ci.cpp \
    src/ci/Drone.cpp \
    src/ci/Model.cpp \
    src/ci/Pane.cpp \
    src/ci/ModelItem.cpp \
    src/ci/NodeEdit.cpp \
    src/includes/includeutils.cpp \
    src/includes/includetree.cpp \
    src/includes/includeextractor.cpp \
    src/includes/includemodifier.cpp \
    src/scrollbars/scrollbarscolorizer.cpp

HEADERS += \
    src/Constants.h \
    src/QtcUtilitiesPlugin.h \
    src/codediscover/CodeDiscover.h \
    src/codediscover/CodeDiscoverConstants.h \
    src/codediscover/CodeDiscoverSettings.h \
    src/codediscover/CodeDiscoverOptionsPage.h \
    src/codediscover/CodeDiscoverWindow.h \
    src/codediscover/CodeDiscoverToolRunner.h \
    src/codediscover/ClassDiagramGenerator.h \
    src/ci/Ci.h \
    src/ci/Drone.h \
    src/ci/Model.h \
    src/ci/Pane.h \
    src/ci/ModelItem.h \
    src/ci/NodeEdit.h \
    src/includes/includeutils.h \
    src/includes/includetree.h \
    src/includes/includeextractor.h \
    src/includes/includemodifier.h \
    src/scrollbars/scrollbarscolorizer.h

TRANSLATIONS += \
    translation/QtcUtilities_ru.ts

OTHER_FILES += \
    QtcUtilities.json.in \
    LICENSE.md \
    README.md \
    TODO.md \
    dist/README.md \
    resources/README.md \
    uncrustify.cfg

PROVIDER = Gres

###### If the plugin can be depended upon by other plugins, this code needs to be outsourced to
###### <dirname>_dependencies.pri, where <dirname> is the name of the directory containing the
###### plugin's sources.

QTC_PLUGIN_NAME = QtcUtilities
QTC_LIB_DEPENDS += \
    cplusplus

QTC_PLUGIN_DEPENDS += \
    coreplugin\
    projectexplorer\
    cpptools\
    debugger

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

###### End _dependencies.pri contents ######

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)

RESOURCES += \
    resources.qrc

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

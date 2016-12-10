DEFINES += QTCUTILITIES_LIBRARY

include(paths.pri)

# QtcUtilities files

SOURCES += \
    src/QtcUtilitiesPlugin.cpp \
    src/clangtools/ClangTools.cpp \
    src/clangtools/AutoCheckEvents.cpp \
    src/clangtools/ToolRunner.cpp \
    src/clangtools/ToolOutputParser.cpp \
    src/clangtools/ToolOptionsPage.cpp \
    src/dockedoutput/DockedOutput.cpp \
    src/organizeincludes/IncludesOrganizer.cpp \
    src/organizeincludes/IncludesOptionsPage.cpp \
    src/organizeincludes/Include.cpp \
    src/organizeincludes/IncludeMap.cpp \
    src/organizeincludes/IncludesExtractor.cpp \
    src/organizeincludes/Document.cpp \
    src/codediscover/CodeDiscover.cpp \
    src/codediscover/CodeDiscoverOptionsPage.cpp \
    src/codediscover/CodeDiscoverWindow.cpp \
    src/codediscover/CodeDiscoverToolRunner.cpp \
    src/codediscover/ClassDiagramGenerator.cpp \
    src/oclint/Oclint.cpp \
    src/oclint/OclintOptionsPage.cpp \
    src/oclint/OclintOutputParser.cpp \
    src/oclint/OclintRunner.cpp \
    src/ci/Ci.cpp \
    src/ci/Drone.cpp \
    src/ci/Model.cpp \
    src/ci/Pane.cpp \
    src/ci/ModelItem.cpp

HEADERS += \
    src/Constants.h \
    src/PluginGlobal.h \
    src/QtcUtilitiesPlugin.h \
    src/clangtools/ClangTools.h \
    src/clangtools/AutoCheckEvents.h \
    src/clangtools/ToolRunner.h \
    src/clangtools/ToolOutputParser.h \
    src/clangtools/ToolOptionsPage.h \
    src/clangtools/CheckTypes.h \
    src/clangtools/ClangToolsConstants.h \
    src/dockedoutput/DockedOutput.h \
    src/organizeincludes/IncludesOrganizer.h \
    src/organizeincludes/IncludesSettings.h \
    src/organizeincludes/IncludesOptionsPage.h \
    src/organizeincludes/IncludesConstants.h \
    src/organizeincludes/Include.h \
    src/organizeincludes/IncludeMap.h \
    src/organizeincludes/IncludesExtractor.h \
    src/organizeincludes/Document.h \
    src/codediscover/CodeDiscover.h \
    src/codediscover/CodeDiscoverConstants.h \
    src/codediscover/CodeDiscoverSettings.h \
    src/codediscover/CodeDiscoverOptionsPage.h \
    src/codediscover/CodeDiscoverWindow.h \
    src/codediscover/CodeDiscoverToolRunner.h \
    src/codediscover/ClassDiagramGenerator.h \
    src/oclint/Oclint.h \
    src/oclint/OclintConstants.h \
    src/oclint/OclintOptionsPage.h \
    src/oclint/OclintOutputParser.h \
    src/oclint/OclintRunner.h \
    src/ci/Ci.h \
    src/ci/Drone.h \
    src/ci/Model.h \
    src/ci/Pane.h \
    src/ci/ModelItem.h

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

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
    src/dockedoutput/DockedOutput.cpp

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
    src/dockedoutput/DockedOutput.h

TRANSLATIONS += \
    translation/QtcUtilities_ru.ts

OTHER_FILES += \
    QtcUtilities.json.in \
    LICENSE.md \
    README.md \
    TODO.md \
    util/README.md \
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
    cpptools

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

###### End _dependencies.pri contents ######

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)

RESOURCES += \
    resources.qrc

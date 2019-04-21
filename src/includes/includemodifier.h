#pragma once

#include <cplusplus/CppDocument.h>

#include <QMap>

class IncludeTree;
class QTextCursor;
class QTextDocument;

class IncludeModifier {
  public:
    explicit IncludeModifier (CPlusPlus::Document::Ptr document);

    void queueDuplicatesRemoval ();
    void queueUpdates (const IncludeTree &tree);
    void executeQueue ();

  private:
    void removeIncludeAt (int line);
    void removeNewLinesBefore (int line);
    void unfoldDocument ();
    bool isGroupRemoved (int line) const;

    CPlusPlus::Document::Ptr document_;
    QTextDocument *textDocument_;
    QVector<int> linesToRemove_;
    QVector<QPair<int, int> > includeGroups_;
};


#pragma once

#include <cplusplus/CppDocument.h>

#include <QMap>

class IncludeTree;
class QTextCursor;
class QTextDocument;

class IncludeTreeApplier {
  public:
    IncludeTreeApplier (CPlusPlus::Document::Ptr document);

    void apply (const IncludeTree &tree);

  private:
    void removeIncludeAt (int line);

    void removeNewLinesBefore (int line, bool isNew);
    int realLine (int line, bool isNew) const;
    QTextCursor cursor (int line, bool isNew = false);
    int lineAfterFirstComment () const;
    void unfoldDocument ();

    CPlusPlus::Document::Ptr document_;
    QTextDocument *textDocument_;
    QMap<int, int> lineChanges_;
};


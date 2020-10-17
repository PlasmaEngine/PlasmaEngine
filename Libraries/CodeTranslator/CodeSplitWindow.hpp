// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class TextEditor;
class ScriptEditor;
class Splitter;
class KeyboardEvent;

/// Interface for the code translator.
struct CodeTranslator
{
  virtual ~CodeTranslator(){};
  virtual void Translate(HashMap<String, String>& files);

  virtual uint GetSourceLexer();
  virtual uint GetDestinationLexer();
};

/// Ui widget to display two code windows
class CodeSplitWindow : public Composite
{
public:
  typedef CodeSplitWindow LightningSelf;
  CodeSplitWindow(Composite* parent);

  void OnKeyDown(KeyboardEvent* event);
  void SetLexers(CodeTranslator* translator);

  ScriptEditor* mSourceText;
  ScriptEditor* mTranslatedText;
  Splitter* mSplitter;

  HandleOf<DocumentResource> mSourceResource;
  String mCommandToRunOnSave;
};

}

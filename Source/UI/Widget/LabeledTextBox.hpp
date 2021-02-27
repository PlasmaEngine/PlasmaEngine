// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

/// A text box with a label to the left. The text box will take up
/// all of the remaining space after the label is sized to its text.
class LabeledTextBox : public Composite
{
public:
  typedef LabeledTextBox LightningSelf;

  LabeledTextBox(Composite* parent);

  /// The text of the label
  String GetLabelText() const;
  void SetLabelText(StringParam labelText);

  /// The text of the text box
  String GetText() const;
  void SetText(StringParam text);

  TextBox* mTextBox;
  Label* mLabel;
};

} // namespace Plasma

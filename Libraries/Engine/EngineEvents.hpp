// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{
class TextEvent : public Event
{
public:
  LightningDeclareType(TextEvent, TypeCopyMode::ReferenceType);
  TextEvent(StringParam text) : Text(text)
  {
  }
  String Text;
};

class TextErrorEvent : public TextEvent
{
public:
  LightningDeclareType(TextErrorEvent, TypeCopyMode::ReferenceType);
  TextErrorEvent(StringParam text, int code) : TextEvent(text), Code(code)
  {
  }
  int Code;
};

class ProgressEvent : public Event
{
public:
  LightningDeclareType(ProgressEvent, TypeCopyMode::ReferenceType);
  ProgressEvent();
  ProgressType::Enum ProgressType;
  String Operation;
  String CurrentTask;
  String ProgressLine;
  float Percentage;
};

class BlockingTaskEvent : public Event
{
public:
  LightningDeclareType(BlockingTaskEvent, TypeCopyMode::ReferenceType);

  BlockingTaskEvent(StringParam taskName = String()) : mTaskName(taskName)
  {
  }
  String mTaskName;
};

} // namespace Plasma

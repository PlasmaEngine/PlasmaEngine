// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

namespace Events
{
LightningDeclareEvent(TranslationError, TranslationErrorEvent);
LightningDeclareEvent(ValidationError, ValidationErrorEvent);
} // namespace Events

/// An error event for when translation fails
class TranslationErrorEvent : public Lightning::EventData
{
public:
  LightningDeclareType(TranslationErrorEvent, Lightning::TypeCopyMode::ReferenceType);
  String GetFormattedMessage(Lightning::MessageFormat::Enum format);

  String mShortMessage;
  String mFullMessage;
  Lightning::CodeLocation mLocation;
};

/// An error even dispatched during validation. Mostly the same as a translation
/// error event, but this also contains a call stack to trace where an error
/// occurred.
class ValidationErrorEvent : public Lightning::EventData
{
  LightningDeclareType(ValidationErrorEvent, Lightning::TypeCopyMode::ReferenceType);
  String GetFormattedMessage(Lightning::MessageFormat::Enum format);

  String mShortMessage;
  String mFullMessage;
  Lightning::CodeLocation mLocation;

  Array<Lightning::CodeLocation> mCallStack;
};

/// Event handler for sending shader compilation errors as well as translation
/// errors.
class ShaderCompilationErrors : public Lightning::EventHandler
{
public:
  ShaderCompilationErrors();

  void SendTranslationError(Lightning::CodeLocation& location, StringParam message);
  void SendTranslationError(Lightning::CodeLocation& location, StringParam shortMsg, StringParam fullMsg);

  void ListenForLightningErrors(Lightning::CompilationErrors& lightningErrors);
  void ListenForTypeParsed(Lightning::CompilationErrors& lightningErrors);
  void ForwardErrorEvent(Lightning::ErrorEvent* e);
  void ForwardGenericEvent(Lightning::EventData* e);

  /// Was an error triggered during compilation (either from translation or
  /// lightning)
  bool mErrorTriggered;
  bool mEmitMultipleErrors;
};

} // namespace Plasma

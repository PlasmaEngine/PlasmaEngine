// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
LightningDefineEvent(TranslationError);
LightningDefineEvent(ValidationError);
} // namespace Events

LightningDefineType(TranslationErrorEvent, builder, type)
{
}

String TranslationErrorEvent::GetFormattedMessage(Lightning::MessageFormat::Enum format)
{
  return mLocation.GetFormattedStringWithMessage(format, mFullMessage);
}

LightningDefineType(ValidationErrorEvent, builder, type)
{
}

String ValidationErrorEvent::GetFormattedMessage(Lightning::MessageFormat::Enum format)
{

  StringBuilder builder;
  // Write the full message with the location
  String msg = BuildString(mShortMessage, "\n", mFullMessage);
  builder.Append(mLocation.GetFormattedStringWithMessage(format, msg));
  // Append all call stack locations to the message (to trace the error)
  for (size_t i = 0; i < mCallStack.Size(); ++i)
  {
    builder.AppendFormat("%s:\n", mCallStack[i].GetFormattedString(format).c_str());
  }

  return builder.ToString();
}

ShaderCompilationErrors::ShaderCompilationErrors()
{
  mErrorTriggered = false;
  mEmitMultipleErrors = false;
}

void ShaderCompilationErrors::SendTranslationError(Lightning::CodeLocation& location, StringParam message)
{
  SendTranslationError(location, message, message);
}

void ShaderCompilationErrors::SendTranslationError(Lightning::CodeLocation& location,
                                                   StringParam shortMsg,
                                                   StringParam fullMsg)
{
  // Check if this is the first error being sent and if not check if we send
  // multiple errors
  if (mErrorTriggered && !mEmitMultipleErrors)
    return;

  mErrorTriggered = true;

  TranslationErrorEvent toSend;
  toSend.EventName = Events::TranslationError;
  toSend.mShortMessage = shortMsg;
  toSend.mFullMessage = fullMsg;
  toSend.mLocation = location;
  EventSend(this, toSend.EventName, &toSend);
}

void ShaderCompilationErrors::ListenForLightningErrors(Lightning::CompilationErrors& lightningErrors)
{
  EventConnect(&lightningErrors, Lightning::Events::CompilationError, &ShaderCompilationErrors::ForwardErrorEvent, this);
}

void ShaderCompilationErrors::ListenForTypeParsed(Lightning::CompilationErrors& lightningErrors)
{
  EventConnect(&lightningErrors, Lightning::Events::TypeParsed, &ShaderCompilationErrors::ForwardGenericEvent, this);
}

void ShaderCompilationErrors::ForwardErrorEvent(Lightning::ErrorEvent* e)
{
  mErrorTriggered = true;
  EventSend(this, e->EventName, e);
}

void ShaderCompilationErrors::ForwardGenericEvent(Lightning::EventData* e)
{
  EventSend(this, e->EventName, e);
}

} // namespace Plasma

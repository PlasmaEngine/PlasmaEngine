// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningSpirVDisassemblerBackend::LightningSpirVDisassemblerBackend()
{
  mTargetEnv = SPV_ENV_UNIVERSAL_1_3;
}

String LightningSpirVDisassemblerBackend::GetExtension()
{
  return "spvtxt";
}

bool LightningSpirVDisassemblerBackend::RunTranslationPass(ShaderTranslationPassResult& inputData,
                                                       ShaderTranslationPassResult& outputData)
{
  mErrorLog.Clear();

  ShaderByteStream& inputByteStream = inputData.mByteStream;
  uint32_t* data = (uint32_t*)inputByteStream.Data();
  size_t wordCount = inputByteStream.WordCount();

  spv_text text;
  spv_diagnostic diagnostic = nullptr;
  uint32_t options =
      SPV_BINARY_TO_TEXT_OPTION_NONE | SPV_BINARY_TO_TEXT_OPTION_INDENT | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES;

  spv_context context = spvContextCreate((spv_target_env)mTargetEnv);
  spv_result_t spvResult = spvBinaryToText(context, data, wordCount, options, &text, &diagnostic);
  spvContextDestroy(context);

  bool success = (spvResult == SPV_SUCCESS);
  if (!success)
  {
    if (diagnostic != nullptr)
      mErrorLog = diagnostic->error;
    return false;
  }

  outputData.mByteStream.Load(text->str, text->length);
  spvTextDestroy(text);
  outputData.mReflectionData = inputData.mReflectionData;

  return success;
}

String LightningSpirVDisassemblerBackend::GetErrorLog()
{
  return mErrorLog;
}

} // namespace Plasma

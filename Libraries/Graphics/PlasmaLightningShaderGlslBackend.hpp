// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

/// Plasma's version of the glsl backend. Needed to
/// set specific options on the compiler for plasma.
class PlasmaLightningShaderGlslBackend : public LightningShaderGlslBackend
{
public:
  PlasmaLightningShaderGlslBackend();

  String GetExtension() override;
  bool RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData) override;
  String GetErrorLog() override;

  int mTargetVersion;
  bool mTargetGlslEs;
  String mErrorLog;
};

} // namespace Plasma

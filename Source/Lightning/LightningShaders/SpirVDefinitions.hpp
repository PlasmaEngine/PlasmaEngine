#pragma once

#include "LightningShadersStandard.hpp"

namespace Plasma
{

//-------------------------------------------------------------------SpirVInstructions
/// Information about spirv instructions. In particular this contains all opcodes by
/// name and opcode as well as information about the arguments for an opcode.
class SpirVInstructions
{
public:
  struct Instruction
  {
    struct Operand
    {
      String mKind;
      String mName;
    };
    String mName;
    int mOpCode;
    Array<Operand> mOperands;
  };

  /// Loads a simplified version of the grammar that only contains simple OpCode + OpName info.
  void LoadSimplifiedDefaults();
  void LoadSimplifiedGlsl450Defaults();
  /// Loads the given json definition of the spirv grammar (from the spirv headers repo).
  void Load(Lightning::JsonValue* jsonRoot);

  bool IsSimplifiedInstructions() const;
  const Instruction* FindInstruction(StringParam opName) const;

private:
  void AddSimpleInstruction(StringParam opName, int opCode);
  void ParseInstructions(Lightning::JsonValue* instructionsJson, HashMap<String, Instruction>& instructions);
  void ParseInstruction(Lightning::JsonValue* instructionJson, HashMap<String, Instruction>& instructions);
  void ParseOperands(Lightning::JsonValue* operandsJson, Array<Instruction::Operand>& operands);

  bool mIsSimplifiedInstructions = false;
  HashMap<String, Instruction> mInstructions;
};

//-------------------------------------------------------------------SpirVDefinitions
/// Utility to load spirv definitions. This is expected to be loaded from the spirv.json definitions,
/// but a simplified version can be auto-populated that contains less information and only the required to run info.
class SpirVDefinitions
{
public:
  /// Loads a simplified version of the grammar that only contains simple OpCode + OpName info.
  void LoadSimplifiedDefaults();
  /// Loads the given json definition of the spirv grammar (from the spirv headers repo).
  void LoadCore(StringParam filePath);
  void LoadExtension(StringParam extensionName, StringParam filePath);

  SpirVInstructions mInstructions;
  HashMap<String, SpirVInstructions> mExtensionInstructions;
};

}//namespace Plasma
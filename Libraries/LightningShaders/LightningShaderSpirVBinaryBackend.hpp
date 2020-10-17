// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class LightningShaderToSpirVContext
{
public:
  LightningShaderToSpirVContext();

  // Get an id for the next IR.
  int GetAndAdvanceId();
  void GenerateId(ILightningShaderIR* ir);

  int FindId(ILightningShaderIR* instruction, bool assertOnPlasma = true);

  ShaderStreamWriter* mStreamWriter;
  ShaderStageInterfaceReflection* mReflectionData;

  // Id mapping of an instruction
  int mId;
  HashMap<ILightningShaderIR*, int> mGeneratedId;
  Array<EntryPointInfo*> mEntryPoints;
  LightningShaderIRFunction* mMain;
};

class LightningShaderSpirVBinaryBackend
{
public:
  ~LightningShaderSpirVBinaryBackend();

  void TranslateType(LightningShaderIRType* type, ShaderStreamWriter& writer);
  void TranslateType(LightningShaderIRType* type,
                     ShaderStreamWriter& writer,
                     ShaderStageInterfaceReflection& reflectionData);

private:
  // Prototype of generating one library with multiple entry points.
  // Not currently used or tested. Also would need to be updated for multiple
  // reflection objects.
  void TranslateLibrary(LightningShaderIRLibrary* library,
                        ShaderStreamWriter& writer,
                        ShaderStageInterfaceReflection& reflectionData);

  // Debugging helper
  void ValidateIdMap(LightningShaderToSpirVContext* context);

  typedef OrderedHashSet<ILightningShaderIR*> IRList;
  typedef OrderedHashSet<LightningShaderIRType*> TypeList;
  typedef OrderedHashSet<LightningShaderIRFunction*> FunctionList;
  typedef OrderedHashSet<LightningShaderIROp*> OpList;
  typedef OrderedHashSet<LightningShaderExtensionImport*> ImportList;
  typedef OrderedHashMap<LightningShaderIRFunction*, LightningShaderIRFunction*> LateBoundFunctionMap;

  /// Helper function to emit the given entry points and their dependencies out
  /// to spirv binary.
  void EmitSpirvBinary(TypeDependencyCollector& collector, LightningShaderToSpirVContext* context);

  void GenerateDummyMain(LightningShaderIRType* type,
                         LightningShaderIRLibrary* library,
                         TypeDependencyCollector& collector,
                         LightningShaderToSpirVContext* context);
  void GenerateGlobalsInitializerFunction(TypeDependencyCollector& collector, LightningShaderToSpirVContext* context);
  void RegisterLateBoundFunctions(LateBoundFunctionMap& lateBoundFunctionMap,
                                  TypeDependencyCollector& collector,
                                  LightningShaderToSpirVContext* context);
  void Clear();

  void AddDecorationCapabilities(TypeDependencyCollector& collector, LightningShaderToSpirVContext* context);
  void AddDecorationCapabilities(EntryPointInfo* entryPoint,
                                 TypeDependencyCollector& collector,
                                 LightningShaderToSpirVContext* context);
  void AddDecorationCapabilities(LightningShaderIROp* decorationOp,
                                 TypeDependencyCollector& collector,
                                 LightningShaderToSpirVContext* context);
  void AddMemberDecorationCapabilities(LightningShaderIROp* memberDecorationOp,
                                       TypeDependencyCollector& collector,
                                       LightningShaderToSpirVContext* context);

  template <typename T>
  void GenerateListIds(OrderedHashSet<T>& input, LightningShaderToSpirVContext* context);
  void GenerateFunctionIds(FunctionList& functions, LightningShaderToSpirVContext* context);
  void GenerateFunctionBlockIds(LightningShaderIRFunction* function, LightningShaderToSpirVContext* context);
  void GenerateBlockLineIds(BasicBlock* block, LightningShaderToSpirVContext* context);

  void WriteHeader(LightningShaderToSpirVContext* context, TypeDependencyCollector& typeCollector);
  void WriteDebug(TypeList& types, LightningShaderToSpirVContext* context);
  void WriteDebug(LightningShaderIRType* type, LightningShaderToSpirVContext* context);
  void WriteDebug(FunctionList& functions, LightningShaderToSpirVContext* context);
  void WriteDebug(LightningShaderIRFunction* function, LightningShaderToSpirVContext* context);
  void WriteDebug(BasicBlock* block, LightningShaderToSpirVContext* context);
  void WriteDebug(OpList& ops, LightningShaderToSpirVContext* context);
  void WriteDebugName(ILightningShaderIR* resultIR, StringParam debugName, LightningShaderToSpirVContext* context);
  void WriteDecorations(LightningShaderToSpirVContext* context);
  void WriteSpecializationConstantBindingDecorations(TypeDependencyCollector& typeCollector,
                                                     LightningShaderToSpirVContext* context);
  LightningShaderIROp* FindSpecialiationConstantCompositeId(LightningShaderIROp* op);

  void WriteTypesGlobalsAndConstants(IRList& typesGlobalsAndConstants, LightningShaderToSpirVContext* context);
  void WriteType(LightningShaderIRType* type, LightningShaderToSpirVContext* context);
  void WriteConstant(LightningShaderIROp* constantOp, LightningShaderToSpirVContext* context);
  void WriteSpecConstant(LightningShaderIROp* constantOp, LightningShaderToSpirVContext* context);
  void WriteGlobal(LightningShaderIROp* globalVarOp, LightningShaderToSpirVContext* context);
  void WriteFunctions(FunctionList& functions, LightningShaderToSpirVContext* context);
  void WriteFunction(LightningShaderIRFunction* function, LightningShaderToSpirVContext* context);

  void WriteBlock(BasicBlock* block, LightningShaderToSpirVContext* context);
  void WriteBlockInstructions(BasicBlock* block,
                              Array<ILightningShaderIR*>& instructions,
                              LightningShaderToSpirVContext* context);
  void WriteIROp(BasicBlock* block, LightningShaderIROp* op, LightningShaderToSpirVContext* context);
  void WriteIROpGeneric(LightningShaderIROp* op, LightningShaderToSpirVContext* context);
  void WriteIROpGenericNoReturnType(LightningShaderIROp* op, LightningShaderToSpirVContext* context);
  void WriteIROpArguments(LightningShaderIROp* op, LightningShaderToSpirVContext* context);
  void WriteIRArguments(Array<ILightningShaderIR*>& mArguments, LightningShaderToSpirVContext* context);
  void WriteIRId(ILightningShaderIR* ir, LightningShaderToSpirVContext* context);
  void WriteImport(LightningShaderExtensionImport* importLibrary, LightningShaderToSpirVContext* context);

  Array<ILightningShaderIR*> mOwnedInstructions;
  Array<EntryPointInfo*> mOwnedEntryPoints;
  LateBoundFunctionMap mExtraLateBoundFunctions;
  LightningShaderIRLibraryRef mLastLibrary;
};

} // namespace Plasma

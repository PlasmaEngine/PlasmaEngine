// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class TypeDependencyCollector
{
public:
  TypeDependencyCollector(LightningShaderIRLibrary* owningLibrary);
  void Collect(LightningShaderIRType* type);

  void Collect(LightningShaderIRFunction* function);
  void Collect(BasicBlock* block);
  void Collect(LightningShaderIROp* op);
  void CollectArguments(LightningShaderIROp* op);
  void Collect(LightningShaderExtensionImport* op);
  void Collect(ILightningShaderIR* instruction);

  void AddTypeReference(LightningShaderIRType* type);
  void AddConstantReference(LightningShaderIROp* op);
  void AddGlobalReference(LightningShaderIROp* op);

  bool IsGlobalStorageClass(spv::StorageClass storageClass);

  OrderedHashSet<LightningShaderIRType*> mReferencedTypes;
  OrderedHashSet<LightningShaderIROp*> mReferencedConstants;
  OrderedHashSet<LightningShaderIROp*> mReferencedGlobals;
  OrderedHashSet<LightningShaderIRFunction*> mReferencedFunctions;
  OrderedHashSet<LightningShaderExtensionImport*> mReferencedImports;
  OrderedHashSet<spv::Capability> mCapabilities;

  // Keep track of the order we found types, constants, and globals. These are
  // all grouped in one section of the module and have to be in the correct
  // usage order (no forward references of ids)
  OrderedHashSet<ILightningShaderIR*> mTypesConstantsAndGlobals;

  // Specifies if an op requires a certain capability that must be added.
  // @JoshD: Parse from the spirv grammar file at some point?
  HashMap<OpType, spv::Capability> mRequiredCapabilities;

  // All global variable initializer functions that need to be called for the
  // given type we processed.
  Array<LightningShaderIRFunction*> mGlobalInitializers;

  // The library of the type we're processing. Needed to find global variables.
  LightningShaderIRLibrary* mOwningLibrary;
};

} // namespace Plasma

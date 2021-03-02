// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{
class Cog;
class CommandManager;
class Composite;
class Editor;
class ObjectEvent;
class Material;
class ShaderDefinition;
class LightningShaderDefinition;
class SimpleLightningShaderGenerator;
class LightningFragment;

// Base class for displaying translated scripts in the editor. Mostly manages a
// lot of helper functions on a LightningCompositor and the actual script editor
// display.
class TranslatedShaderScriptEditor : public ScriptEditor
{
public:
  typedef TranslatedShaderScriptEditor LightningSelf;
  TranslatedShaderScriptEditor(Composite* parent);
  ~TranslatedShaderScriptEditor();

  // Updates the script editor with the translated text from OnTranslate
  void Build();
  void OnBuild(Event* e);
  // Function meant to be implemented by derived classes that translate as
  // needed
  virtual String OnTranslate();

  // Helper functions
  // void CompileAndTranslateFragments(SimpleLightningShaderGenerator&
  // shaderGenerator); void TranslateMaterial(SimpleLightningShaderGenerator&
  // shaderGenerator, Material* material);

  // The Connect macro doesn't seem to work when trying to connect from a
  // derived type on a base class function, hence this function allows easy
  // connection on a resource type being modified.
  template <typename ResourceType>
  void ListenForModified(ResourceType* resource)
  {
    ConnectThisTo(resource, Events::ResourceModified, OnBuild);
  }

  SimpleLightningShaderGenerator* mShaderGenerator;
};

// Translates a given fragment. As long as this editor is open on
// save/modification of the fragment the script editor will be updated.
class FragmentFileTranslatorScriptEditor : public TranslatedShaderScriptEditor
{
public:
  typedef FragmentFileTranslatorScriptEditor LightningSelf;
  FragmentFileTranslatorScriptEditor(Composite* parent);

  void SetResource(LightningFragment* fragment);
  String OnTranslate() override;

  LightningFragment* mFragment;
};

// Builds the lightning script representing a composite from a material. As long as
// this editor is open on save/modification of the fragment the script editor
// will be updated.
class LightningCompositorScriptEditor : public TranslatedShaderScriptEditor
{
public:
  typedef LightningCompositorScriptEditor LightningSelf;
  LightningCompositorScriptEditor(Composite* parent);

  void SetResource(Material* material);
  String OnTranslate() override;

  Material* mMaterial;
};

DeclareEnum4(TranslationDisplayMode, Pixel, Geometry, Vertex, Full);
// Translates the lightning script of a material. As long as this editor is open
// on save/modification of the fragment the script editor will be updated.
class TranslatedLightningCompositorScriptEditor : public TranslatedShaderScriptEditor
{
public:
  typedef TranslatedLightningCompositorScriptEditor self_type;
  TranslatedLightningCompositorScriptEditor(Composite* parent);

  void SetResource(Material* material);
  String OnTranslate() override;

  void SetDisplayMode(TranslationDisplayMode::Enum displayMode);

  Material* mMaterial;
  TranslationDisplayMode::Enum mDisplayMode;
};

class BaseSplitScriptEditor : public Composite
{
public:
  typedef BaseSplitScriptEditor LightningSelf;
  BaseSplitScriptEditor(Composite* parent);

  void Setup();
  virtual void SetTranslatedEditor();
  virtual void SaveCheck();
  virtual void Build();
  // virtual void BuildFinalShader(ShaderTypeTranslation& shaderResult);

  void SetLexer(uint lexer);
  void OnSaveCheck(SavingEvent* e);
  void OnBuild(Event* e);
  void OnLeftMouseDown(MouseEvent* e);

  // CodeRangeMapping* FindRange(int positionWithinParent, CodeRangeMapping*
  // current);

  ScriptEditor* mSourceEditor;
  Splitter* mSplitter;
  TranslatedShaderScriptEditor* mTranslatedEditor;
};

// A simple script editor to view a translated shader and a source fragment.
// Currently used to debug line number mappings (Cleanup later).
class FragmentSplitScriptEditor : public BaseSplitScriptEditor
{
public:
  typedef FragmentSplitScriptEditor LightningSelf;
  FragmentSplitScriptEditor(Composite* parent);

  virtual void SetTranslatedEditor() override;
  virtual void SaveCheck() override;
  virtual void Build() override;
  // virtual void BuildFinalShader(ShaderTypeTranslation& shaderResult)
  // override;

  void SetResource(LightningFragment* fragment);

  LightningFragment* mFragment;
};

// A simple script editor to view a translated shader and a source material.
// Currently used to debug line number mappings (Cleanup later).
class MaterialSplitScriptEditor : public BaseSplitScriptEditor
{
public:
  typedef MaterialSplitScriptEditor LightningSelf;
  MaterialSplitScriptEditor(Composite* parent);

  void SetTranslatedEditor() override;
  void Build() override;
  // void BuildFinalShader(ShaderTypeTranslation& shaderResult) override;

  void SetResource(Material* material);
  void SetDisplayMode(TranslationDisplayMode::Enum displayMode);

  TranslationDisplayMode::Enum mDisplayMode;
  Material* mMaterial;
};

// A simple class to listen for events. Ideally this should be a component.
class CodeTranslatorListener : public EventObject
{
public:
  typedef CodeTranslatorListener LightningSelf;
  CodeTranslatorListener();

  void OnComposeLightningMaterial(ObjectEvent* e);
  void OnTranslateLightningFragment(ObjectEvent* e);
  void OnTranslateLightningPixelFragment(ObjectEvent* e);
  void OnTranslateLightningGeometryFragment(ObjectEvent* e);
  void OnTranslateLightningVertexFragment(ObjectEvent* e);

  void OnTranslateLightningFragmentWithLineNumbers(ObjectEvent* e);
  void OnTranslateLightningPixelFragmentWithLineNumbers(ObjectEvent* e);
  void OnTranslateLightningVertexFragmentWithLineNumbers(ObjectEvent* e);
};

void BindCodeTranslatorCommands(Cog* configCog, CommandManager* commands);

}

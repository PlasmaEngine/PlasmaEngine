// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
DefineEvent(ColorSchemeChanged);
}

void GetStringsTextConfig(HandleParam instance, Property* property, Array<String>& strings)
{
  ColorScheme* colorScheme = instance.Get<ColorScheme*>();
  forRange (auto& scheme, colorScheme->mAvailableSchemes.All())
    strings.PushBack(scheme.first);
}

LightningDefineType(ColorScheme, builder, type)
{
  type->HandleManager = LightningManagerId(PointerManager);
  type->Add(new MetaOperations());

  LightningBindGetterSetterProperty(ActiveScheme)->Add(new EditorIndexedStringArray(GetStringsTextConfig));

  LightningBindFieldProperty(Default);
  LightningBindFieldProperty(Background);
  LightningBindFieldProperty(Selection);
  LightningBindFieldProperty(LineSelection);
  LightningBindFieldProperty(Comment);
  LightningBindFieldProperty(StringLiteral);
  LightningBindFieldProperty(Number);
  LightningBindFieldProperty(Keyword);
  LightningBindFieldProperty(Operator);
  // until we have a lightning lexer setup that colors the class/function names
  // these fields should not be exposed as a property since they currently have
  // no effect
  LightningBindField(ClassName);
  LightningBindField(FunctionName);
  LightningBindFieldProperty(SpecialWords);
  LightningBindFieldProperty(Error);
  LightningBindFieldProperty(Whitespace);
  LightningBindFieldProperty(Gutter);
  LightningBindFieldProperty(GutterText);
  LightningBindFieldProperty(Link);
  LightningBindFieldProperty(TextMatchIndicator);
  LightningBindFieldProperty(TextMatchHighlight);
  LightningBindFieldProperty(TextMatchOutlineAlpha)->Add(new EditorSlider());

  LightningBindGetterSetterProperty(SaveName);
  LightningBindMethodProperty(Save)->AddAttribute(FunctionAttributes::cInvalidatesObject);
}

void ColorPropertyChanged(BoundType* meta, Property* property, ObjPtr instance, AnyParam oldValue, AnyParam newValue)
{
  ColorScheme* colorScheme = (ColorScheme*)instance;
  colorScheme->Modified();
}

ColorScheme::ColorScheme()
{
  Default = FloatColorRGBA(0xDA, 0xDA, 0xDA, 0xFF);
  Background = FloatColorRGBA(0x21, 0x1E, 0x1E, 0xFF);
  Selection = FloatColorRGBA(0x45, 0x40, 0x40, 0xFF);
  LineSelection = FloatColorRGBA(0x35, 0x30, 0x30, 0xFF);
  Comment = FloatColorRGBA(0x00, 0x80, 0x00, 0xFF);
  StringLiteral = FloatColorRGBA(0xAD, 0x93, 0x61, 0xFF);
  Number = FloatColorRGBA(0xDA, 0xDA, 0xDA, 0xFF);
  Keyword = FloatColorRGBA(0x69, 0x69, 0xFA, 0xFF);
  Operator = FloatColorRGBA(0xDA, 0xDA, 0xDA, 0xFF);
  ClassName = FloatColorRGBA(0x98, 0x4A, 0x4A, 0xFF);
  FunctionName = FloatColorRGBA(0x98, 0x4A, 0x4A, 0xFF);
  SpecialWords = FloatColorRGBA(0xC1, 0xC1, 0x44, 0xFF);
  Error = FloatColorRGBA(0xC8, 0x00, 0x00, 0xFF);
  Whitespace = FloatColorRGBA(0x60, 0x60, 0x60, 0xFF);
  Gutter = FloatColorRGBA(0x21, 0x1E, 0x1E, 0xFF);
  GutterText = FloatColorRGBA(0x90, 0x90, 0x90, 0xFF);
  Link = FloatColorRGBA(0x64, 0x64, 0xFA, 0xFF);
  TextMatchIndicator = FloatColorRGBA(0x40, 0xE0, 0xD0, 0xBF);
  TextMatchHighlight = FloatColorRGBA(0x40, 0xE0, 0xD0, 0x32);
  TextMatchOutlineAlpha = 0x69;
}

ColorScheme::~ColorScheme()
{
}

// Hex order of RGB is different from the order of the
// ByteColor so swap the order for saving / loading
uint ConvertByteColorHexOrder(const uint& inputColor)
{
  return ByteColorRGBA(((::byte*)&inputColor)[2], ((::byte*)&inputColor)[1], ((::byte*)&inputColor)[0], 0xFF);
}

void SerializeRGBColor(Serializer& stream, cstr fieldName, uint& colorValue)
{
  if (stream.GetMode() == SerializerMode::Loading)
  {
    StringRange stringRange;
    bool serialized = stream.StringField("Color", fieldName, stringRange);
    if (serialized)
    {
      u64 fullInt = (u64)ReadHexString(stringRange);
      colorValue = ConvertByteColorHexOrder((uint)fullInt);
    }
  }
  else
  {
    const uint colorTripleSize = 6;
    char texBuffer[colorTripleSize + 1];
    WriteToHexSize(texBuffer, colorTripleSize + 1, colorTripleSize, ConvertByteColorHexOrder(colorValue), true);
    StringRange stringRange = texBuffer;
    stream.StringField("Color", fieldName, stringRange);
  }
}

#define SerializeRGBColorName(fieldName) SerializeRGBColor(stream, #fieldName, fieldName)

void ColorScheme::Serialize(Serializer& stream)
{
  SerializeNameDefault(Default, Vec4::cZero);
  SerializeNameDefault(Background, Vec4::cZero);
  SerializeNameDefault(Selection, Vec4::cZero);
  SerializeNameDefault(LineSelection, Vec4::cZero);
  SerializeNameDefault(Comment, Vec4::cZero);
  SerializeNameDefault(StringLiteral, Vec4::cZero);
  SerializeNameDefault(Number, Vec4::cZero);
  SerializeNameDefault(Keyword, Vec4::cZero);
  SerializeNameDefault(Operator, Vec4::cZero);
  SerializeNameDefault(ClassName, Vec4::cZero);
  SerializeNameDefault(FunctionName, Vec4::cZero);
  SerializeNameDefault(SpecialWords, Vec4::cZero);
  SerializeNameDefault(Error, Vec4::cZero);
  SerializeNameDefault(Whitespace, Vec4::cZero);
  SerializeNameDefault(Directive, Vec4::cZero);
  SerializeNameDefault(Gutter, Vec4::cZero);
  SerializeNameDefault(GutterText, Vec4::cZero);
  SerializeNameDefault(Link, Vec4::cZero);
  SerializeNameDefault(TextMatchIndicator, Vec4::cZero);
  SerializeNameDefault(TextMatchHighlight, Vec4::cZero);
  SerializeNameDefault(TextMatchOutlineAlpha, 0.0f);
}

String ColorScheme::GetActiveScheme()
{
  return mActiveName;
}

void ColorScheme::SetActiveScheme(StringParam name)
{
  Load(name);

  UpdateConfig();

  // If they are not a developer they can not
  // edit the built in color schemes
  if (PL::gEngine->GetConfigCog()->has(DeveloperConfig) == NULL)
    mFilePath = String();

  Modified();
}

void ColorScheme::SetSaveName(StringParam name)
{
  // Clear location for custom
  mFilePath = String();
  mSaveName = name;
}

void ColorScheme::UpdateConfig()
{
  TextEditorConfig* textConfig = PL::gEngine->GetConfigCog()->has(TextEditorConfig);
  textConfig->ColorScheme = mActiveName;
  SaveConfig();
}

void ColorScheme::Modified()
{
  ObjectEvent e(this);
  this->DispatchEvent(Events::ColorSchemeChanged, &e);
}

void ColorScheme::Save()
{
  if (mSaveName.Empty())
  {
    DoNotifyWarning("Invalid Name", "Please set a name for the color scheme");
    return;
  }

  String userSchemeDirectory = FilePath::Combine(GetUserApplicationDirectory(), "ColorSchemes");
  if (mFilePath.Empty())
    mFilePath = userSchemeDirectory;

  CreateDirectoryAndParents(mFilePath);
  String filename = BuildString(mSaveName, ".data");
  String fullPath = FilePath::Combine(mFilePath, filename);
  SaveToDataFile(*this, fullPath, DataFileFormat::Text);

  UpdateConfig();

  // After saving a color scheme we need enumerate and add the new user color
  // schemes
  Enumerate(userSchemeDirectory);
  // Set our active scheme to the new saved scheme
  SetActiveScheme(mSaveName);
}

void ColorScheme::Load(StringParam name)
{
  if (mActiveName == name)
    return;

  String path = mAvailableSchemes.FindValue(name, String());
  if (!path.Empty())
  {
    mFilePath = FilePath::GetDirectoryPath(path);
    mActiveName = FilePath::GetFileNameWithoutExtension(path);
    LoadFromDataFile(*this, path);
  }
}

ColorScheme* GetColorScheme()
{
  ColorScheme* colorScheme = ColorScheme::GetInstance();
  if (colorScheme->mAvailableSchemes.Empty())
    colorScheme->LoadSchemes();
  return colorScheme;
}

void ColorScheme::LoadSchemes()
{
  // Load schemes in data directory and user directory
  Cog* configCog = PL::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);
  TextEditorConfig* textConfig = configCog->has(TextEditorConfig);

  String userColorSchemeDirectory = FilePath::Combine(GetUserDocumentsApplicationDirectory(), "ColorSchemes");
  CreateDirectoryAndParents(userColorSchemeDirectory);
  Enumerate(userColorSchemeDirectory);

  // Default color schemes are loaded second to overwrite user schemes using the
  // same name
  String defaultColorSchemeDirectory = FilePath::Combine(mainConfig->DataDirectory, "ColorSchemes");
  Enumerate(defaultColorSchemeDirectory);

  Load(textConfig->ColorScheme);
}

void ColorScheme::Enumerate(StringParam directoryPath)
{
  // Add color scheme files in directory
  FileRange filesInDirectory(directoryPath);
  for (; !filesInDirectory.Empty(); filesInDirectory.PopFront())
  {
    String filename = FilePath::Combine(directoryPath, filesInDirectory.Front());
    String name = FilePath::GetFileNameWithoutExtension(filename);
    mAvailableSchemes.SortedInsertOrOverride(name, filename);
  }
}

} // namespace Plasma

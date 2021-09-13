// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

namespace Events
{
	DeclareEvent(FolderLocationUpdated);
	DeclareEvent(ConfigChangeApplied);
} // namespace Events

class StringChangeEvent : public Event
{
public:
	LightningDeclareType(StringChangeEvent, TypeCopyMode::ReferenceType);

	StringChangeEvent(String oldValue, String newValue)
		: OldValue(oldValue), NewValue(newValue)
	{}

	String OldValue;
	String NewValue;
};

struct FolderLocationStyle
{
	String mFont;
	Vec4 mTextColor, mBackgroundColor, mHoverColor, mClickedColor;
	Vec4 mButtonHoverColor, mButtonClickedColor;
};

class FolderLocation : public Composite
{
public:
	LightningDeclareType(FolderLocation, TypeCopyMode::ReferenceType);

	FolderLocation(Composite* parent);

	typedef String(*GetStringFn)();
	typedef void(*SetStringFn)(String s);

	// function ptr callbacks for getting and setting field from config object
	GetStringFn GetConfigValue;
	SetStringFn SetConfigValue;

	String mLabel;
	String mFileDialogFilterDescription;
	String mConfigSavedEventLabel;
	FolderLocationStyle mStyle;

	void Create();

	// overwrites the config value with the currently-entered text
	void ApplyChange();

	// resets the current text to whatever is in the config file
	void Reset();

	// resets the text and also shows an error message
	void ResetWithError(String error);

protected:
	void OnLocationTextChanged(Event* e);
	void OnLocationTextSubmit(Event* e);
	void OnBrowseLocation(Event* e);
	void OnBrowseLocationSelected(OsFileSelection* e);

	HandleOf<ToolTip> mToolTip;
	TextBoxButton* mLocation;
};

} // namespace Plasma

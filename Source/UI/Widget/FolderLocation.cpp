// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
    DefineEvent(FolderLocationUpdated);
    DefineEvent(ConfigChangeApplied);
} // namespace Events

LightningDefineType(StringChangeEvent, builder, type)
{
}

LightningDefineType(FolderLocation, builder, type)
{
}

FolderLocation::FolderLocation(Composite* parent) :
    Composite(parent)
{
    SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 5)));
}

void FolderLocation::Create()
{
    Text* text = new Text(this, mStyle.mFont, 11);
    text->SetText(mLabel);
    text->SetColor(mStyle.mTextColor);

    new Spacer(this, SizePolicy::Fixed, Pixels(0, -9));

    mLocation = new TextBoxButton(this, "OpenFolderIcon");
    mLocation->SetText(GetConfigValue());
    mLocation->SetStyle(TextBoxStyle::Modern);
    mLocation->SetEditable(true);
    mLocation->mButton->mBackgroundColor = ToByteColor(mStyle.mBackgroundColor);
    mLocation->mButton->mBackgroundHoverColor = ToByteColor(mStyle.mButtonHoverColor);
    mLocation->mButton->mBackgroundClickedColor = ToByteColor(mStyle.mButtonClickedColor);
    ConnectThisTo(mLocation, Events::TextChanged, OnLocationTextChanged);
    ConnectThisTo(mLocation, Events::TextSubmit, OnLocationTextSubmit);
    ConnectThisTo(mLocation->mButton, Events::ButtonPressed, OnBrowseLocation);
}

void FolderLocation::OnLocationTextChanged(Event* e)
{
    // If there's an active tooltip then destroy it (the user is changing the text
    // right now)
    ToolTip* toolTip = mToolTip;
    if (toolTip != nullptr)
        toolTip->Destroy();
}

void FolderLocation::OnLocationTextSubmit(Event* e)
{
    StringChangeEvent stringEvent(GetConfigValue(), mLocation->GetText());
    GetDispatcher()->Dispatch(Events::FolderLocationUpdated, &stringEvent);
}

void FolderLocation::OnBrowseLocation(Event* e)
{
    // Set up the callback for when folder is selected
    const String CallBackEvent = "LocationCallback";
    if (!GetDispatcher()->IsConnected(CallBackEvent, this))
        ConnectThisTo(this, CallBackEvent, OnBrowseLocationSelected);

    // Open the open file dialog
    FileDialogConfig* config = FileDialogConfig::Create();
    config->EventName = CallBackEvent;
    config->CallbackObject = this;
    config->Title = "Select a folder";
    config->AddFilter(mFileDialogFilterDescription, "*.none");
    config->DefaultFileName = mLocation->GetText();
    config->StartingDirectory = mLocation->GetText();
    config->Flags |= FileDialogFlags::Folder;
    PL::gEngine->has(OsShell)->SaveFile(config);
}

void FolderLocation::OnBrowseLocationSelected(OsFileSelection* e)
{
    if (e->Files.Size() > 0)
    {
        String path = FilePath::GetDirectoryPath(e->Files[0]);
        mLocation->SetText(path);

        StringChangeEvent stringEvent(GetConfigValue(), mLocation->GetText());
        GetDispatcher()->Dispatch(Events::FolderLocationUpdated, &stringEvent);
    }
}

void FolderLocation::ApplyChange()
{
    // Grab the old path and update to the new one
    String oldPath = GetConfigValue();
    SetConfigValue(mLocation->GetText());
    SaveConfig();

    // callback to SettingsMenu, so it can do specific stuff (like move downloads)
    StringChangeEvent stringEvent(oldPath, mLocation->GetText());
    GetDispatcher()->Dispatch(Events::ConfigChangeApplied, &stringEvent);
    
    Event toSend;
    PL::gEngine->GetConfigCog()->DispatchEvent(mConfigSavedEventLabel, &toSend);
}

void FolderLocation::Reset()
{
    // They hit cancel, set the location text back to the config's value
    mLocation->SetText(GetConfigValue());
}

void FolderLocation::ResetWithError(String error)
{
    mLocation->SetText(GetConfigValue());

    ToolTip* toolTip = new ToolTip(mLocation, error);
    mToolTip = toolTip;
    // Don't have the tooltip destroyed when the mouse leaves the text box
    toolTip->SetDestroyOnMouseExit(false);
    // Queue up an action to destroy the tooltip after a little bit
    ActionSequence* seq = new ActionSequence(toolTip);
    seq->Add(new ActionDelay(5.0f));
    seq->Add(new CallAction<Widget, &Widget::Destroy>(toolTip));
    return;
}

} // namespace Plasma

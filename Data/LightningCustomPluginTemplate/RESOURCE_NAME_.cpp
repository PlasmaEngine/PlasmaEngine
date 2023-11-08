#include "RESOURCE_NAME_Precompiled.hpp"

//***************************************************************************
LightningDefineType(RESOURCE_NAME_, builder, type)
{
  // This is required for component binding
  LightningBindDestructor();
  LightningBindConstructor();
  LightningBindMethod(Initialize);
  
  // Note: All event connection methods must be bound
  LightningBindMethod(OnLogicUpdate);

  // Using Property at the end is the same as the [Property] attribute
  // You could also use ->AddAttribute after the bind macro
  LightningBindMethod(Speak);
  LightningBindFieldProperty(mLives);
  LightningBindGetterSetterProperty(Health);
}

//***************************************************************************
RESOURCE_NAME_::RESOURCE_NAME_()
{
  Lightning::Console::WriteLine("RESOURCE_NAME_::RESOURCE_NAME_ (Constructor)");
  // Initialize our default values here (we automatically zero the memory first)
  // In the future we'll support a newer compiler with member initialization
  mHealth = 100.0f;
  mLives = 9;
}

//***************************************************************************
RESOURCE_NAME_::~RESOURCE_NAME_()
{
  Lightning::Console::WriteLine("RESOURCE_NAME_::~RESOURCE_NAME_ (Destructor)");
  // Always check for null if you are intending
  // to destroy any cogs that you 'own'
}

//***************************************************************************
void RESOURCE_NAME_::Initialize(PlasmaEngine::CogInitializer* initializer)
{
  Lightning::Console::WriteLine("RESOURCE_NAME_::Initialize");
  
  PlasmaConnectThisTo(this->GetSpace(), "LogicUpdate", "OnLogicUpdate");
}

//***************************************************************************
void RESOURCE_NAME_::OnLogicUpdate(PlasmaEngine::UpdateEvent* event)
{
  // Do we have a Model component?
  PlasmaEngine::Model* model = this->GetOwner()->has(PlasmaEngine::Model);
  if (model != nullptr)
    Lightning::Console::WriteLine("We have a Model!");
  
  // Send our own update event
  // We could also replace this with LightningEvent to send basic events
  // Note: LightningAllocate should be used for any type that is
  // typically allocated within Lightning, such as a CastFilter
  Lightning::HandleOf<RESOURCE_NAME_Event> toSend = LightningAllocate(RESOURCE_NAME_Event);
  toSend->mLives = mLives;
  this->GetOwner()->DispatchEvent("RESOURCE_NAME_Update", toSend);
}

//***************************************************************************
Lightning::String RESOURCE_NAME_::Speak()
{
  Lightning::String text("Hello World");
  Lightning::Console::WriteLine(text);
  return text;
}

//***************************************************************************
float RESOURCE_NAME_::GetHealth()
{
  return mHealth;
}

//***************************************************************************
void RESOURCE_NAME_::SetHealth(float value)
{
  if (value < 0)
    value = 0;
  else if (value > 100)
    value = 100;
  
  mHealth = value;
}

//***************************************************************************
LightningDefineType(RESOURCE_NAME_Event, builder, type)
{
  // This is required for event binding
  LightningBindDestructor();
  LightningBindConstructor();
  
  LightningBindFieldProperty(mLives);
}

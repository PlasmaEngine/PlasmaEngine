#pragma once

// An example component being bound to the engine
class RESOURCE_NAME_ : public PlasmaEngine::LightningComponent
{
public:
  LightningDeclareType(Lightning::TypeCopyMode::ReferenceType);
  
  RESOURCE_NAME_();
  ~RESOURCE_NAME_();
  
  void Initialize(PlasmaEngine::CogInitializer* initializer);
  
  void OnLogicUpdate(PlasmaEngine::UpdateEvent* event);
  
  // A method that we want to expose to script
  Lightning::String Speak();
  
  // A field that we want to expose to script
  int mLives;
  
  // A getter/setter that we want to expose to script
  float GetHealth();
  void SetHealth(float value);
  
private:
  float mHealth;
};

// An example of a custom event that we can send
class RESOURCE_NAME_Event : public PlasmaEngine::LightningEvent
{
public:
  LightningDeclareType(Lightning::TypeCopyMode::ReferenceType);
  
  int mLives;
};

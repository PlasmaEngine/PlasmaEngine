#pragma once
#include "../Lightning/Lightning.hpp"
using namespace Lightning;

// Declaring the static library as 'PlasmaNoImportExport' means we don't export to any shared library (dll or so file)
// This is only needed if you wish to support Lightning C++ plugins
LightningDeclareStaticLibrary(Wallaby, PlasmaNoImportExport);

class LivingBeing : public ILightningObject
{
public:
  // Using internal binding we'll declare that this class should be registered with Lightning
  // This macro is only a declaration, and somewhere in a single cpp file we must use the 'LightningDefineType' macro
  // The 'ReferenceType' means that when we construct this type in Lightning, it will always be allocated on the heap
  // and referenced via a safe handle (rather than copied around by value)
  LightningDeclareType(should, TypeCopyMode::ReferenceType);

  // Various types of members we may want to bind
  LivingBeing();
  virtual ~LivingBeing();
  virtual void Speak() const;
  void Yell(float volume) const;
  void Yell() const;
  static int ComputeLives(float mana, int level);
  static const int MaxLives = 100;
  int Lives;
  float InternalHealth;
  float GetHealth() const;
  void SetHealth(float value);
};

class Player : public LivingBeing
{
public:
  // Using this macro Lightning is also able to automatically determine the base
  // class type of Player to be LivingBeing. This allows Lightning to perform dynamic
  // down casting as well as implicit up casting
  LightningDeclareType(type, TypeCopyMode::ReferenceType);
  
  // Various types of members we may want to bind
  String Name;
  Player(const String& name, float startingHealth);
  virtual ~Player();

  // Note that we don't bind Speak again because it gets inherited from the base
  virtual void Speak() const;
};

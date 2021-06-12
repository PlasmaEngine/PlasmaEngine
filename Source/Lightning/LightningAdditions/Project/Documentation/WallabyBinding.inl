#include "Wallaby.hpp"

LightningDefineStaticLibrary(Wallaby)
{
  // We have to initialize all types that we have bound to our library (automatically any type to Wallaby)
  // Ideally we could use pre-main or automatic registration, but there's a major issue where
  // compilers will automatically remove "unreferenced" classes, even if they are referenced
  // by globals/pre-main initializations. This method ensures that all classes will be properly bound
  LightningInitializeType(LivingBeing);
  LightningInitializeType(Player);
}

// This allows us to define all the members on LivingBeing
// The 'builder' and 'type' members are only there to let the user know they can do builder->... or type->...
// If you wanted to change the name that gets bound to Lightning, use LightningFullDefineType
LightningDefineType(LivingBeing, builder, type)
{
  // The 'LightningNoNames' macro is simply a way of saying that there are no parameter names for the argument types
  // Lightning supports named parameter calling, so feel free to provide them

  // We should generally always bind a destructor and constructor,
  // especially if this is a dervied class and the base is constructable
  LightningBindDestructor();
  LightningBindConstructor();

  // This simplified binding macro allows us to quickly bind instance and static methods on our own class
  // This only works if there are no overloads of the method
  LightningBindMethod(Speak);

  // The binding templates/macros can automatically determine if you're binding a static or instance member function
  // You can also use this to bind global functions to a class, even when they weren't originally defined within the class
  // The 'LightningNoOverload' lets the binding know that there are no overloads of the same name
  // Otherwise, we'd have to pass in the type/signature of the member function in parentheses
  LightningFullBindMethod(builder, type, &LivingBeing::ComputeLives, LightningNoOverload, "ComputeLives", "mana, level");

  // Bind both overloads of Yell
  LightningFullBindMethod(builder, type, &LivingBeing::Yell, (void (LivingBeing::*)(float) const), "Speak", "volume");
  LightningFullBindMethod(builder, type, &LivingBeing::Yell, (void (LivingBeing::*)() const), "Speak", LightningNoNames);

  // Simple macros for binding readable and writable data members (fields)
  LightningBindField(Lives);

  // Lightning does not have the concept of 'const' (therefore we remove all consts from bound C++ members)
  // It is up to us to be very careful here and bind const members as 'Get' only
  LightningFullBindField(builder, type, &LivingBeing::MaxLives, "MaxLives", PropertyBinding::Get);

  // We can specially bind getters and setters in C++ as a single property in Lightning
  LightningBindGetterSetter(Health);

  // The above binding is the exact same as doing the following
  //LightningFullBindGetterSetter(builder, type, &LivingBeing::GetHealth, LightningNoOverload, &LivingBeing::SetHealth, LightningNoOverload, "Health");
}

LightningDefineType(Player, builder, type)
{
  // Be sure to always pass the correct types in to all the bindings
  // Do NOT pass LivingBeing, for example, and avoid copy pasting from other bindings!

  // Even though we only have a non-overloaded constructor, we unfortunately cannot
  // detect the argument types automatically for constructors due to a limitation in C++
  // The argument types must be explicitly passed in
  LightningFullBindConstructor(builder, type, Player, "name, startingHealth", const String&, float);
  LightningBindDestructor();

  LightningBindField(Name);
  
  // Note that we don't bind Speak again because it gets inherited from the base
}

#include "Lightning.hpp"

namespace Lightning
{
  //***************************************************************************
  Composition::Composition()
  {
  }
  
  //***************************************************************************
  Composition::~Composition()
  {
  }

  //***************************************************************************
  Composition* Composition::GetBaseComposition()
  {
    return nullptr;
  }

  //***************************************************************************
  void Composition::ClearComponents()
  {
    this->Components.Clear();
  }
}

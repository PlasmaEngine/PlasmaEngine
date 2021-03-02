// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(ParticleAnimator, builder, type)
{
  PlasmaBindDocumented();
  PlasmaBindTag(Tags::Particle);
}

ParticleAnimator::ParticleAnimator() : mGraphicsSpace(nullptr)
{
}

void ParticleAnimator::Initialize(CogInitializer& initializer)
{
  mGraphicsSpace = GetSpace()->has(GraphicsSpace);
}

} // namespace Plasma

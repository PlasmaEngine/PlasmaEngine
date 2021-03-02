// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

/// Returns the basic native type corresponding to the specified lightning type (may
/// be null), else nullptr
NativeType* LightningTypeToBasicNativeType(Lightning::Type* lightningType);

/// Returns the lightning type corresponding to the specified basic native type (may
/// be null), else nullptr
Lightning::Type* BasicNativeTypeToLightningType(NativeType* nativeType);
Lightning::Type* BasicNativeTypeToLightningType(NativeTypeId nativeTypeId);

/// Returns a variant containing the stored value of the specified any if the
/// any's stored value is a basic native type, else Variant()
Variant ConvertBasicAnyToVariant(const Any& anyValue);
/// Returns an any containing the stored value of the specified variant if the
/// variant's stored value is a basic native type, else Any()
Any ConvertBasicVariantToAny(const Variant& variantValue);

} // namespace Plasma

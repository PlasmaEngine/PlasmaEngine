// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{
	// Forward Declarations.
	class UiWidget;

	/// Layouts are in charge of calling UpdateTransform on all children, regardless
	/// of whether or not they ignore layouts.
	class UiButton : public Component
	{
	public:
		/// Meta Initialization.
		LightningDeclareType(UiLayout, TypeCopyMode::ReferenceType);

		/// Component Interface.
		void Serialize(Serializer& stream) override;
		void Initialize(CogInitializer& initializer) override;
	}
}
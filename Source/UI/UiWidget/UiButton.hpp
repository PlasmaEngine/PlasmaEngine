// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{
	class UiButton : public UiWidget
	{
	public:
		/// Meta Initialization.
		LightningDeclareType(UiButton, TypeCopyMode::ReferenceType);

		/// Component Interface.
		void Serialize(Serializer& stream) override;
		void Initialize(CogInitializer& initializer) override;
	};
}
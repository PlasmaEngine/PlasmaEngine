// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{
	DeclareEnum3(UiButtonState, MouseOver, Pressed, Idle);

	class UiButton : public UiWidget
	{
	public:
		/// Meta Initialization.
		LightningDeclareType(UiButton, TypeCopyMode::ReferenceType);

		// Color
		Real4 MouseHoverColor = Real4(1);
		Real4 MouseDownColor = Real4(1);

		// States
		UiButtonState::Enum State;

		/// Component Interface.
		void Serialize(Serializer& stream) override;
		void Initialize(CogInitializer& initializer) override;

		//Getters and Setters
		Real4 GetMouseHoverColor();
		void SetMouseHoverColor(Real4 color);

		Real4 GetMouseDownColor();
		void SetMouseDownColor(Real4 color);

		UiButtonState::Enum GetState();
		void SetState(UiButtonState::Enum state);

	private:
		// Button stuff...
		void OnMouseEnter();
		void OnMouseExit();
		void OnLeftMouseDown();
		void OnLeftMouseUp();
	};
}
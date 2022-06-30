// MIT Licensed (see LICENSE.md).
#pragma once


namespace Plasma
{
	namespace Events
	{
		DeclareEvent(UiButtonStateChanged);
	}

	DeclareEnum3(UiButtonState, MouseOver, Pressed, Idle);

	class UiButtonStateChangedEvent : public Event
	{
	public:
		/// Meta Initialization.
		LightningDeclareType(UiButtonStateChangedEvent, TypeCopyMode::ReferenceType);

		UiButtonState::Enum FromState = UiButtonState::Idle;
		UiButtonState::Enum ToState = UiButtonState::Idle;

		UiButtonState::Enum GetFromState();
		UiButtonState::Enum GetToState();
	};

	class UiButton : public UiWidget
	{
	public:
		/// Meta Initialization.
		LightningDeclareType(UiButton, TypeCopyMode::ReferenceType);

		// Visuals
		Real4 MouseHoverColor = Real4(1);
		Real4 MouseDownColor = Real4(1);

		// States
		UiButtonState::Enum State = UiButtonState::Idle;

		// Component Interface.
		void Serialize(Serializer& stream) override;
		void Initialize(CogInitializer& initializer) override;

		//Getters and Setters
		Real4 GetMouseHoverColor();
		void SetMouseHoverColor(Real4 color);

		Real4 GetMouseDownColor();
		void SetMouseDownColor(Real4 color);

		UiButtonState::Enum GetState();

	private:
		Real4 mOriginalColor;
		void SetState(UiButtonState::Enum state);
		void OnMouseEnter(ViewportMouseEvent* e);
		void OnMouseExit(ViewportMouseEvent* e);
		void OnLeftMouseDown(ViewportMouseEvent* e);
		void OnLeftMouseUp(ViewportMouseEvent* e);
		bool ShouldRunInEditMode();
	};
}
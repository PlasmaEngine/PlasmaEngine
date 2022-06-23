// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
	LightningDefineType(UiButton, builder, type)
	{
		PlasmaBindDocumented();
		PlasmaBindComponent();
		PlasmaBindSetup(SetupMode::DefaultSerialization);

		LightningBindGetterSetterProperty(State);
		LightningBindGetterSetterProperty(MouseHoverColor);
		LightningBindGetterSetterProperty(MouseDownColor);
	}

	void UiButton::Serialize(Serializer& stream)
	{
		UiWidget::Serialize(stream);
	}

	void UiButton::Initialize(CogInitializer& initializer)
	{
		UiWidget::Initialize(initializer);
	}

	Real4 UiButton::GetMouseHoverColor()
	{
		return MouseHoverColor;
	}

	void UiButton::SetMouseHoverColor(Real4 color)
	{
		MouseHoverColor = color;
	}

	Real4 UiButton::GetMouseDownColor()
	{
		return MouseDownColor;
	}

	void UiButton::SetMouseDownColor(Real4 color)
	{
		MouseDownColor = color;
	}

	UiButtonState::Enum UiButton::GetState()
	{
		return State;
	}

	void UiButton::SetState(UiButtonState::Enum state)
	{
		State = state;
	}

	void UiButton::OnMouseEnter()
	{
	}

	void UiButton::OnMouseExit()
	{
	}

	void UiButton::OnLeftMouseDown()
	{
	}

	void UiButton::OnLeftMouseUp()
	{
	}
}

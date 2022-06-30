// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
	namespace Events
	{
		DefineEvent(UiButtonStateChanged);
	}

	LightningDefineType(UiButtonStateChangedEvent, builder, type)
	{
		PlasmaBindDocumented();
		LightningBindGetterProperty(ToState);
		LightningBindGetterProperty(FromState);
	}

	LightningDefineType(UiButton, builder, type)
	{
		PlasmaBindDocumented();
		PlasmaBindComponent();
		PlasmaBindSetup(SetupMode::DefaultSerialization);
		PlasmaBindInterface(UiWidget);

		PlasmaBindEvent(Events::UiButtonStateChanged, UiButtonStateChangedEvent);
		LightningBindGetterSetterProperty(MouseHoverColor);
		LightningBindGetterSetterProperty(MouseDownColor);
		LightningBindGetterSetterProperty(MouseDetectionMode);
		LightningBindGetter(State);
	}

	void UiButton::Serialize(Serializer& stream)
	{
		UiWidget::Serialize(stream);
	}

	void UiButton::Initialize(CogInitializer& initializer)
	{
		if (GetOwner()->has(UiWidget))
		{
			mTransform = GetOwner()->has(Transform);
			mArea = GetOwner()->has(Area);
			DoNotifyWarning("Invalid component placement", "UiButton is not compatible with UiWidget");
			GetOwner()->ForceRemoveComponent(this);
			return;
		}

		UiWidget::Initialize(initializer);

		if (MouseDetectionMode == UiButtonMouseDetectionMode::OnEnter)
		{
			ConnectThisTo(GetOwner(), Events::MouseEnter, OnMouseEnter);
			ConnectThisTo(GetOwner(), Events::MouseExit, OnMouseExit);
		}
		else if (MouseDetectionMode == UiButtonMouseDetectionMode::OnEnterHierarchy)
		{
			ConnectThisTo(GetOwner(), Events::MouseEnterHierarchy, OnMouseEnter);
			ConnectThisTo(GetOwner(), Events::MouseExitHierarchy, OnMouseExit);
		}
		else
		{
			ConnectThisTo(GetOwner(), Events::MouseHover, OnMouseEnter);
			ConnectThisTo(GetOwner(), Events::MouseExitHierarchy, OnMouseExit);
		}

		ConnectThisTo(GetOwner(), Events::LeftMouseDown, OnLeftMouseDown);
		ConnectThisTo(GetOwner(), Events::LeftMouseUp, OnLeftMouseUp);
	}

	UiButtonState::Enum UiButtonStateChangedEvent::GetFromState()
	{
		return FromState;
	}

	UiButtonState::Enum UiButtonStateChangedEvent::GetToState()
	{
		return ToState;
	}

	UiButtonMouseDetectionMode::Enum UiButton::GetMouseDetectionMode()
	{
		return MouseDetectionMode;
	}

	void UiButton::SetMouseDetectionMode(UiButtonMouseDetectionMode::Enum detection)
	{
		MouseDetectionMode = detection;
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
		auto lastState = State;
		State = state;
		UiButtonStateChangedEvent e;
		e.FromState = lastState;
		e.ToState = state;
		GetOwner()->DispatchEvent(Events::UiButtonStateChanged, &e);
	}

	void UiButton::OnMouseEnter(ViewportMouseEvent* e)
	{
		// TODO: See if there is a need or want to cascade color down to children
		// add an enum for that and apply hover and click colors to the mHierarchyColor...
		mOriginalColor = UiWidget::mLocalColor;
		UiWidget::mLocalColor = MouseHoverColor;
		SetState(UiButtonState::MouseOver);
	}

	void UiButton::OnMouseExit(ViewportMouseEvent* e)
	{
		UiWidget::mLocalColor = mOriginalColor;
		SetState(UiButtonState::Idle);
	}

	void UiButton::OnLeftMouseDown(ViewportMouseEvent* e)
	{
		UiWidget::mLocalColor = MouseDownColor;
		SetState(UiButtonState::Pressed);
	}

	void UiButton::OnLeftMouseUp(ViewportMouseEvent* e)
	{
		UiWidget::mLocalColor = MouseHoverColor;
		SetState(UiButtonState::MouseOver);
	}
}

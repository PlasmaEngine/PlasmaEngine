// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

// For all properties that affect the perspective transforms
#define SetPerspectiveProperty(member, value)                                                                          \
  if (value == member)                                                                                                 \
    return;                                                                                                            \
  member = value;                                                                                                      \
  mDirtyPerspective = true;

namespace Plasma
{
	namespace Events
	{
		DefineEvent(CameraUpdate);
		DefineEvent(CameraDestroyed);
	} // namespace Events

	LightningDefineType(Camera, builder, type)
	{
		PlasmaBindComponent();
		PlasmaBindSetup(SetupMode::DefaultSerialization);
		PlasmaBindDependency(Transform);
		PlasmaBindDocumented();

		LightningBindGetterSetterProperty(NearPlane);
		LightningBindGetterSetterProperty(FarPlane);
		LightningBindGetterSetterProperty(PerspectiveMode)->AddAttribute(PropertyAttributes::cInvalidatesObject);
		LightningBindGetterSetterProperty(FieldOfView)
										  ->Add(new EditorSlider(45, 135, 1))
										  ->PlasmaFilterEquality(mPerspectiveMode, PerspectiveMode::Enum, PerspectiveMode::Perspective);
		LightningBindGetterSetterProperty(Aperture)
										  ->Add(new EditorSlider(1.4f, 22.0f, 0.1f));
		LightningBindGetterSetterProperty(FocalDistance);
		LightningBindGetterSetterProperty(ShutterSpeed)
										  ->Add(new EditorSlider(1.0f, 2000.0f, 10.0f));
		LightningBindGetterSetterProperty(ISO)
										  ->Add(new EditorSlider(50.0f, 1600.0f, 50.0f));
		LightningBindGetterSetterProperty(Size)->PlasmaFilterEquality(mPerspectiveMode, PerspectiveMode::Enum, PerspectiveMode::Orthographic);

		LightningBindGetter(CameraViewportCog);
		LightningBindGetter(WorldTranslation);
		LightningBindGetter(WorldDirection);
		LightningBindGetter(WorldUp);

		LightningBindMethod(GetFrustum);
	}

	void Camera::Serialize(Serializer& stream)
	{
		SerializeNameDefault(mNearPlane, 0.5f);
		SerializeNameDefault(mFarPlane, 100.0f);
		SerializeEnumNameDefault(PerspectiveMode, mPerspectiveMode, PerspectiveMode::Perspective);
		SerializeNameDefault(mFieldOfView, 45.0f);
		SerializeNameDefault(mSize, 20.0f);
		SerializeNameDefault(mAperture, 16.0f);
		SerializeNameDefault(mFocalDistance, 22.0f);
		SerializeNameDefault(mShutterSpeed, 250.0f);
		SerializeNameDefault(mISO, 100.0f);
	}

	void Camera::Initialize(CogInitializer& initializer)
	{
		mTransform = GetOwner()->has(Transform);

		mDirtyView = true;
		mDirtyPerspective = true;
		mViewportInterface = nullptr;
		mVisibilityId = static_cast<uint>(-1);
		mRenderQueuesDataNeeded = false;
	}

	void Camera::OnDestroy(uint flags)
	{
		// Tell CameraViewport, if there is one, to remove this camera
		ObjectEvent event(this);
		DispatchEvent(Events::CameraDestroyed, &event);
	}

	void Camera::TransformUpdate(TransformUpdateInfo& info)
	{
		mDirtyView = true;
		ObjectEvent event(this);
		DispatchEvent(Events::CameraUpdate, &event);
	}

	float Camera::GetNearPlane()
	{
		return mNearPlane;
	}

	void Camera::SetNearPlane(float nearPlane)
	{
		nearPlane = Math::Max(nearPlane, 0.01f);
		float farPlane = Math::Max(mFarPlane, nearPlane + 0.1f);
		SetPerspectiveProperty(mNearPlane, nearPlane);
		SetPerspectiveProperty(mFarPlane, farPlane);
	}

	float Camera::GetFarPlane()
	{
		return mFarPlane;
	}

	void Camera::SetFarPlane(float farPlane)
	{
		farPlane = Math::Max(farPlane, mNearPlane + 0.1f);
		SetPerspectiveProperty(mFarPlane, farPlane);
	}

	PerspectiveMode::Enum Camera::GetPerspectiveMode()
	{
		return mPerspectiveMode;
	}

	void Camera::SetPerspectiveMode(PerspectiveMode::Enum perspectiveMode)
	{
		SetPerspectiveProperty(mPerspectiveMode, perspectiveMode);
	}

	float Camera::GetFieldOfView()
	{
		return mFieldOfView;
	}

	void Camera::SetFieldOfView(float fieldOfView)
	{
		fieldOfView = Math::Clamp(fieldOfView, 10.0f, 170.0f);
		SetPerspectiveProperty(mFieldOfView, fieldOfView);
	}

	float Camera::GetSize()
	{
		return mSize;
	}

	void Camera::SetSize(float size)
	{
		size = Math::Max(size, 0.01f);
		SetPerspectiveProperty(mSize, size);
	}

	float Camera::GetAperture()
	{
		return mAperture;
	}

	void Camera::SetAperture(float aperture)
	{
		mAperture = aperture;
	}

	float Camera::GetFocalDistance()
	{
		return mFocalDistance;
	}

	void Camera::SetFocalDistance(float focalDistance)
	{
		mFocalDistance = focalDistance;
	}

	float Camera::GetShutterSpeed()
	{
		return mShutterSpeed;
	}

	void Camera::SetShutterSpeed(float shutterSpeed)
	{
		mShutterSpeed = shutterSpeed;
	}

	float Camera::GetISO()
	{
		return mISO;
	}

	void Camera::SetISO(float iso)
	{
		mISO = iso;
	}

	HandleOf<Cog> Camera::GetCameraViewportCog()
	{
		if (mViewportInterface != nullptr)
			return mViewportInterface->GetOwner();
		return nullptr;
	}

	Vec3 Camera::GetWorldTranslation()
	{
		return mTransform->GetWorldTranslation();
	}

	Vec3 Camera::GetWorldDirection()
	{
		return Multiply(mTransform->GetWorldRotation(), -Vec3::cZAxis);
	}

	Vec3 Camera::GetWorldUp()
	{
		return Multiply(mTransform->GetWorldRotation(), Vec3::cYAxis);
	}

	float Camera::GetAspectRatio()
	{
		return mAspectRatio;
	}

	void Camera::SetAspectRatio(float aspectRatio)
	{
		SetPerspectiveProperty(mAspectRatio, aspectRatio);
	}

	Mat4 Camera::GetViewTransform()
	{
		if (mDirtyView == false)
			return mWorldToView;

		Mat4 rotation = ToMatrix4(mTransform->GetWorldRotation());

		Mat4 translation;
		translation.Translate(-mTransform->GetWorldTranslation());

		mWorldToView = rotation.Transposed() * translation;
		mDirtyView = false;
		return mWorldToView;
	}

	Mat4 Camera::GetPerspectiveTransform()
	{
		if (mDirtyPerspective == false)
			return mViewToPerspective;

		if (mPerspectiveMode == PerspectiveMode::Perspective)
		{
			BuildPerspectiveTransformPlasma(
				mViewToPerspective, Math::DegToRad(mFieldOfView), mAspectRatio, mNearPlane, mFarPlane);
			PL::gRenderer->BuildPerspectiveTransform(
				mViewToApiPerspective, Math::DegToRad(mFieldOfView), mAspectRatio, mNearPlane, mFarPlane);
		}
		else
		{
			BuildOrthographicTransformPlasma(mViewToPerspective, mSize, mAspectRatio, mNearPlane, mFarPlane);
			PL::gRenderer->
				BuildOrthographicTransform(mViewToApiPerspective, mSize, mAspectRatio, mNearPlane, mFarPlane);
		}

		mDirtyPerspective = false;
		return mViewToPerspective;
	}

	Mat4 Camera::GetApiPerspectiveTransform()
	{
		// Will update both perspective transforms if dirty
		GetPerspectiveTransform();
		return mViewToApiPerspective;
	}

	void Camera::GetViewData(ViewBlock& block)
	{
		ErrorIf(mViewportInterface == nullptr, "Invalid Camera to get view data from.");

		SetAspectRatio(mViewportInterface->GetAspectRatio());

		block.mWorldToView = GetViewTransform();
		block.mViewToPerspective = GetPerspectiveTransform();

		Mat4 apiPerspective = GetApiPerspectiveTransform();
		block.mPlasmaPerspectiveToApiPerspective = apiPerspective * block.mViewToPerspective.SafeInverted();

		block.mNearPlane = mNearPlane;
		block.mFarPlane = mFarPlane;

		Vec2 viewportSize = mViewportInterface->GetViewportSize();
		block.mViewportSize = viewportSize;
		block.mInverseViewportSize = Vec2(1.0f / viewportSize.x, 1.0f / viewportSize.y);

		block.mEyePosition = GetWorldTranslation();
		block.mEyeDirection = GetWorldDirection();
		block.mEyeUp = GetWorldUp();
		block.mFieldOfView = mFieldOfView;
		block.mOrthographicSize = mSize;
		block.mOrthographic = mPerspectiveMode == PerspectiveMode::Orthographic;

		block.mCameraId = GetOwner()->GetId().ToUint64();
	}

	Frustum Camera::GetFrustum(float aspect) const
	{
		Vec3 position = mTransform->GetWorldTranslation();
		Mat3 rotation = ToMatrix3(mTransform->GetWorldRotation());

		Frustum f;

		if (mPerspectiveMode == PerspectiveMode::Perspective)
			f.Generate(position, rotation, mNearPlane, mFarPlane, aspect, Math::DegToRad(mFieldOfView));
		else
			f.Generate(position - rotation.BasisZ() * mNearPlane,
			           -rotation.BasisZ(),
			           rotation.BasisY(),
			           Vec3(mSize * 0.5f * aspect, mSize * 0.5f, mFarPlane));

		return f;
	}
} // namespace Plasma

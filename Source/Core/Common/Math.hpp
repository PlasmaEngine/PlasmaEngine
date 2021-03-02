// MIT Licensed (see LICENSE.md).
#pragma once

#include "Reals.hpp"
#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "Matrix2.hpp"
#include "Matrix3.hpp"
#include "Matrix4.hpp"
#include "Quaternion.hpp"
#include "EulerAngles.hpp"
#include "IntVector2.hpp"
#include "IntVector3.hpp"
#include "IntVector4.hpp"

namespace Math
{

/// Creates a skew symmetric matrix from the given 3D vector. Multiplying a
/// vector by this matrix is equivalent to the cross product using the input
/// vector.
PlasmaShared Matrix3 SkewSymmetric(Vec3Param vec3);

/// Converts a quaternion to an axis-angle pair (in radians). Axis is stored in
/// the Vector4's xyz and the angle is stored in the w.
PlasmaShared Vector4 ToAxisAngle(QuatParam quaternion);
PlasmaShared void ToAxisAngle(QuatParam quaternion, Vec4Ptr axisAngle);

/// Converts a quaternion to an axis-angle pair (in radians).
PlasmaShared void ToAxisAngle(QuatParam quaternion, Vec3Ptr axis, real* radians);

/// Convert a 3x3 matrix to a set of Euler angles (in radians). The desired
/// order of the rotations is expected to be in the given Euler angle structure.
PlasmaShared EulerAngles ToEulerAngles(Mat3Param matrix, EulerOrders::Enum order = EulerOrders::XYZs);
PlasmaShared void ToEulerAngles(Mat3Param matrix, EulerAnglesPtr eulerAngles);

/// Convert a 4x4 matrix to a set of Euler angles in radians. The desired order
/// of the rotations is expected to be in the given Euler angle structure.
PlasmaShared EulerAngles ToEulerAngles(Mat4Param matrix, EulerOrders::Enum order = EulerOrders::XYZs);
PlasmaShared void ToEulerAngles(Mat4Param matrix, EulerAnglesPtr eulerAngles);

/// Convert a quaternion to a set of Euler angles (in radians). The desired
/// order of the rotations is expected to be in the given Euler angle structure.
PlasmaShared EulerAngles ToEulerAngles(QuatParam quaternion, EulerOrders::Enum order = EulerOrders::XYZs);
PlasmaShared void ToEulerAngles(QuatParam quaternion, EulerAnglesPtr eulerAngles);

/// Converts from Vector3 to Vector2, removing the z component of the Vector3.
PlasmaShared Vector2 ToVector2(Vec3Param v3);

/// Converts from Vector2 to Vector3, adding the given z component.
PlasmaShared Vector3 ToVector3(Vec2Param v, real z = real(0.0));

/// Converts from Vector4 to Vector3, removing the w component.
PlasmaShared Vector3 ToVector3(Vec4Param v);

/// Converts from IntVec3 to Vector3.
PlasmaShared Vector3 ToVector3(IntVec3Param v);

/// Converts from Vector3 to Vector4, adding the given w component.
PlasmaShared inline Vec4 ToVector4(Vec3Param v, real w = real(0.0))
{
  return Vec4(v.x, v.y, v.z, w);
}

// Convert from a IntVec2 to a Vec2
PlasmaShared inline Vec2 ToVec2(IntVec2 v)
{
  return Vec2(real(v.x), real(v.y));
}

// Convert from a Vec2 to a IntVec2 standard float to int conversion
PlasmaShared inline IntVec2 ToIntVec2(Vec2Param vec2)
{
  return IntVec2(int(vec2.x), int(vec2.y));
}
PlasmaShared inline IntVec2 ToIntVec2(Vec3Param vec3)
{
  return IntVec2(int(vec3.x), int(vec3.y));
}

/// Converts an axis-angle pair to a 3x3 (in radians). Axis is stored in the
/// Vector4's xyz and the angle is stored in the w. Axis is assumed to be
/// normalized.
PlasmaShared Matrix3 ToMatrix3(Vec4Param axisAngle);
PlasmaShared void ToMatrix3(Vec4Param axisAngle, Mat3Ptr matrix);

/// Converts an axis-angle pair to a 3x3 matrix (in radians). Axis is assumed to
/// be normalized.
PlasmaShared Matrix3 ToMatrix3(Vec3Param axis, real radians);
PlasmaShared void ToMatrix3(Vec3Param axis, real radians, Mat3Ptr matrix);

/// Convert a set of Euler angles to a 3x3 matrix (in radians).
PlasmaShared Matrix3 ToMatrix3(EulerAnglesParam eulerAngles);
PlasmaShared void ToMatrix3(EulerAnglesParam eulerAngles, Mat3Ptr matrix);
/// Convert a set of Euler angles to a 3x3 matrix (in radians).
PlasmaShared Matrix3 ToMatrix3(real xRadians, real yRadians, real zRadians);

/// Convert a 3x3 matrix to a 2x2 matrix. Simply copies the 3x3 matrix's upper
/// 2x2 matrix (rotation & scale) to the 2x2 matrix.
PlasmaShared Matrix2 ToMatrix2(Mat3Param matrix3);
PlasmaShared void ToMatrix2(Mat3Param matrix3, Mat2Ptr matrix2);

/// Convert a 4x4 matrix to a 3x3 matrix. Simply copies the 4x4 matrix's upper
/// 3x3 matrix (rotation & scale) to the 3x3 matrix.
PlasmaShared Matrix3 ToMatrix3(Mat4Param matrix4);
PlasmaShared void ToMatrix3(Mat4Param matrix4, Mat3Ptr matrix3);

/// Converts a quaternion to a 3x3 rotation matrix (in radians).
PlasmaShared Matrix3 ToMatrix3(QuatParam quaternion);
PlasmaShared void ToMatrix3(QuatParam quaternion, Mat3Ptr matrix);

PlasmaShared Matrix3 ToMatrix3(Vec3Param facing);
PlasmaShared Matrix3 ToMatrix3(Vec3Param facing, Vec3Param up);
PlasmaShared Matrix3 ToMatrix3(Vec3Param facing, Vec3Param up, Vec3Param right);

/// Convert a set of Euler angles to a 4x4 matrix (in radians).
PlasmaShared Matrix4 ToMatrix4(EulerAnglesParam eulerAngles);
PlasmaShared void ToMatrix4(EulerAnglesParam eulerAngles, Mat4Ptr matrix);

/// Convert a 3x3 matrix to a 4x4 matrix. Simply copies the 3x3 matrix's values
/// into the rotational part of the 4x4 matrix.
PlasmaShared Matrix4 ToMatrix4(Mat3Param matrix3);
PlasmaShared void ToMatrix4(Mat3Param matrix3, Mat4Ptr matrix4);

/// Converts a quaternion to a 4x4 rotation matrix (in radians).
PlasmaShared Matrix4 ToMatrix4(QuatParam quaternion);
PlasmaShared void ToMatrix4(QuatParam quaternion, Mat4Ptr matrix);

/// Converts an axis-angle pair to a quaternion (in radians). Axis is stored in
/// the Vector4's xyz and the angle is stored in the w. Axis is assumed to be
/// normalized.
PlasmaShared Quaternion ToQuaternion(Vec4Param axisAngle);
PlasmaShared void ToQuaternion(Vec4Param axisAngle, QuatPtr quaternion);

/// Converts an axis-angle pair to a quaternion (in radians). Axis is assumed to
/// be normalized.
PlasmaShared Quaternion ToQuaternion(Vec3Param axis, real radians);
PlasmaShared void ToQuaternion(Vec3Param axis, real radians, QuatPtr quaternion);

/// Convert a set of Euler angles to a quaternion (in radians).
PlasmaShared Quaternion ToQuaternion(Vec3Param eulerVector);

/// Convert a set of Euler angles to a quaternion (in radians).
PlasmaShared Quaternion ToQuaternion(EulerAnglesParam eulerAngles);
PlasmaShared void ToQuaternion(EulerAnglesParam eulerAngles, QuatPtr quaternion);

/// Converts a 3x3 matrix to a quaternion (in radians).
PlasmaShared Quaternion ToQuaternion(Mat3Param matrix);
PlasmaShared void ToQuaternion(Mat3Param matrix, QuatPtr quaternion);

/// Converts a 4x4 matrix to a quaternion (in radians).
PlasmaShared Quaternion ToQuaternion(Mat4Param matrix);
PlasmaShared void ToQuaternion(Mat4Param matrix, QuatPtr quaternion);

PlasmaShared Quaternion ToQuaternion(Vec3Param facing, Vec3Param up);
PlasmaShared Quaternion ToQuaternion(Vec3Param facing, Vec3Param up, Vec3Param right);
/// Generates a quaternion from the x,y,z axis angles.
PlasmaShared Quaternion ToQuaternion(real x, real y, real z);
/// Generates the quaternion that rotates start to end.
PlasmaShared Quaternion RotationQuaternionBetween(Vec3Param start, Vec3Param end);

/// Generates a vector perpendicular to the given vector. The result is not
/// normalized.
PlasmaShared Vec2 GeneratePerpendicularVector(Vec2Param input);
/// Generates a vector perpendicular to the given vector. The result is not
/// normalized.
PlasmaShared Vec3 GeneratePerpendicularVector(Vec3Param input);

/// Generates a set of orthonormal vectors from the given vectors, modifying u
/// and v.
PlasmaShared void GenerateOrthonormalBasis(Vec3Param w, Vec3Ptr u, Vec3Ptr v);

/// Doesn't blow up on plasma vectors
PlasmaShared void DebugGenerateOrthonormalBasis(Vec3Param w, Vec3Ptr u, Vec3Ptr v);

/// Converts a 32-bit float into a compressed 16-bit floating point value;
/// referenced from Insomniac Games math library.
PlasmaShared half ToHalf(float value);

/// Converts a 16-bit compressed floating point value back into a 32-bit float;
/// referenced from Insomniac Games math library.
PlasmaShared float ToFloat(half value);

/// Computes the angle about the z-axis between the vector and the x-axis
PlasmaShared real Angle2D(Vec3Param a);

/// Rotate a vector towards another changing at most maxAngle (radians).
PlasmaShared Vector2 RotateTowards(Vec2Param a, Vec2Param b, real maxAngle);
/// Rotate a vector towards another changing at most maxAngle (radians).
PlasmaShared Vector3 RotateTowards(Vec3Param a, Vec3Param b, real maxAngle);
/// Rotate a quaternion towards another changing at most maxAngle (radians).
PlasmaShared Quat RotateTowards(QuatParam a, QuatParam b, real maxAngle);

/// Same as RotateTowards except this function deals correctly with
/// invalid vectors. Used for binding to scripting languages.
PlasmaShared Vector2 SafeRotateTowards(Vec2Param a, Vec2Param b, real maxAngle);
/// Same as RotateTowards except this function deals correctly with
/// invalid vectors. Used for binding to scripting languages.
PlasmaShared Vector3 SafeRotateTowards(Vec3Param a, Vec3Param b, real maxAngle);

/// Get the rotation angle between two vectors in radians.
PlasmaShared real SignedAngle(Vec3Param a, Vec3Param b, Vec3Param up);

/// Rotate a vector about an axis by the given angle.
PlasmaShared Vector3 RotateVector(Vec3Param a, Vec3Param axis, real radians);

/// Converts Euler degrees to a quaternion.
PlasmaShared Quat EulerDegreesToQuat(Vec3Param eulerDegrees);

/// Converts a quaternion to Euler degrees.
PlasmaShared Vector3 QuatToEulerDegrees(QuatParam rotation);

PlasmaShared float Luminance(Vector3 linearColor);

/// Perlin Noise
PlasmaShared float PerlinNoise(float persistence, float frequency, float limit, float x, float y);
PlasmaShared float PerlinNoise(float persistence, float frequency, float limit, float x);

/// White Noise
PlasmaShared float WhiteNoise(int x, int y);
PlasmaShared float WhiteNoise(int x);

/// Smooth Noise
PlasmaShared float SmoothNoise(int x, int y);
PlasmaShared float SmoothNoise(int x);

/// Interpolated Noise
PlasmaShared float InterpolatedNoise(float x, float y);
PlasmaShared float InterpolatedNoise(float x);
} // namespace Math

// MIT Licensed (see LICENSE.md).
#pragma once

namespace Math
{

struct IndexDim3Policy
{
  real& operator()(Matrix3& A, size_t row, size_t col)
  {
    return A(row, col);
  }

  real& operator()(Vector3& v, size_t i)
  {
    return v[i];
  }

  size_t GetDimension(Vector3& v)
  {
    return 3;
  }
};

struct IndexDim4Policy
{
  real& operator()(Matrix4& A, size_t row, size_t col)
  {
    return A(row, col);
  }

  real& operator()(Vector4& v, size_t i)
  {
    return v[i];
  }

  size_t GetDimension(Vector4& v)
  {
    return 4;
  }
};

struct GenericDimIndexPolicy
{
  template <typename MatrixType>
  real& operator()(MatrixType& A, size_t row, size_t   col)
  {
    return A(row, col);
  }

  template <typename VectorType>
  real& operator()(VectorType& v, size_t i)
  {
    return v[i];
  }

  template <typename VectorType>
  size_t GetDimension(VectorType& v)
  {
    return v.GetSize();
  }
};

} // namespace Math

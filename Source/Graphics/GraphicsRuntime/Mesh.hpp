// MIT Licensed (see LICENSE.md).

#pragma once

namespace Plasma
{

class VertexSemanticRange
{
public:
  typedef VertexSemanticRange self_type;
  typedef VertexSemantic::Enum value_type;
  typedef VertexSemantic::Enum FrontResult;

  VertexSemanticRange()
  {
  }
  VertexSemanticRange(const FixedVertexDescription& fixedDesc);

  bool Empty();
  VertexSemantic::Enum Front();
  void PopFront();

  FixedVertexDescription mFixedDesc;
  uint mCurrentIndex;
};

/// Vertex data and attribute semantics for defining data that can be uploaded
/// to the gpu.
class VertexBuffer : public SafeId32
{
public:
  LightningDeclareType(VertexBuffer, TypeCopyMode::ReferenceType);

  VertexBuffer();
  ~VertexBuffer();

  /// Adds an attribute to the definition of the vertices that are to be stored.
  /// Add the attributes in the order that they should be stored in memory on a
  /// vertex.
  void AddAttribute(VertexSemantic::Enum semantic, VertexElementType::Enum elementType, uint elementCount);

  /// Returns a range of attribute semantics in the order that they were added.
  VertexSemanticRange GetAttributes();

  /// Adds data as bytes to the buffer, data is expected in the order of the
  /// attributes, one vertex after another.
  void AddByte(int value);
  /// Adds data as bytes to the buffer, data is expected in the order of the
  /// attributes, one vertex after another.
  void AddByte(IntVec2 value);
  /// Adds data as bytes to the buffer, data is expected in the order of the
  /// attributes, one vertex after another.
  void AddByte(IntVec3 value);
  /// Adds data as bytes to the buffer, data is expected in the order of the
  /// attributes, one vertex after another.
  void AddByte(IntVec4 value);

  /// Adds data as shorts to the buffer, data is expected in the order of the
  /// attributes, one vertex after another.
  void AddShort(int value);
  /// Adds data as shorts to the buffer, data is expected in the order of the
  /// attributes, one vertex after another.
  void AddShort(IntVec2 value);
  /// Adds data as shorts to the buffer, data is expected in the order of the
  /// attributes, one vertex after another.
  void AddShort(IntVec3 value);
  /// Adds data as shorts to the buffer, data is expected in the order of the
  /// attributes, one vertex after another.
  void AddShort(IntVec4 value);

  /// Adds data as floats to the buffer, data is expected in the order of the
  /// attributes, one vertex after another.
  void AddReal(real value);
  /// Adds data as floats to the buffer, data is expected in the order of the
  /// attributes, one vertex after another.
  void AddReal(Vec2 value);
  /// Adds data as floats to the buffer, data is expected in the order of the
  /// attributes, one vertex after another.
  void AddReal(Vec3 value);
  /// Adds data as floats to the buffer, data is expected in the order of the
  /// attributes, one vertex after another.
  void AddReal(Vec4 value);

  /// Gets the data of an attribute of a vertex, returns values of 0 if read is
  /// invalid.
  Vec4 GetVertexData(uint vertexIndex, VertexSemantic::Enum semantic);
  /// Gets the data of an attribute of a vertex, throws exception if attribute
  /// info doesn't match or read is invalid.
  Vec4 GetVertexData(uint vertexIndex, VertexSemantic::Enum semantic, VertexElementType::Enum type, uint count);

  /// Returns false if GetVertexData() would throw an exception with the same
  /// arguments.
  bool IsValidVertexData(uint vertexIndex, VertexSemantic::Enum semantic, VertexElementType::Enum type, uint count);

  /// Clears all added attributes from the vertex definition so they can be
  /// redefined.
  void ClearAttributes();

  /// Clears all added vertex data so new data can be added.
  void ClearData();

  /// Returns the type that is used to store the given attribute, throws
  /// exception if the attribute is not in the vertex definition.
  VertexElementType::Enum GetElementType(VertexSemantic::Enum semantic);

  /// Returns the number of elements stored for the given attribute, throws
  /// exception if the attribute is not in the vertex definition.
  uint GetElementCount(VertexSemantic::Enum semantic);

  /// Returns the number of vertices that have a complete set of data stored.
  uint GetVertexCount();

  // Internal

  void Grow(uint minExtra);
  VertexAttribute GetAttribute(VertexSemantic::Enum semantic);
  uint GetElementSize(VertexElementType::Enum type);
  void ReadVertexData(::byte* vertexData, VertexAttribute& attribute, Vec4& output);
  void CopyTo(VertexBuffer& target);

  template <typename T>
  void WriteData(const T& value);

  template <typename T>
  T GetData(uint vertexIndex, VertexSemantic::Enum semantic);

  FixedVertexDescription mFixedDesc;
  ::byte* mData;
  uint mDataCapacity;
  uint mDataSize;
};

template <typename T>
void VertexBuffer::WriteData(const T& value)
{
  uint valueSize = sizeof(T);
  if (mDataCapacity - mDataSize < valueSize)
    Grow(valueSize);

  memcpy(mData + mDataSize, (const void*)&value, valueSize);
  mDataSize += valueSize;
}

template <typename T>
T VertexBuffer::GetData(uint vertexIndex, VertexSemantic::Enum semantic)
{
  T value = T();

  VertexAttribute attribute = GetAttribute(semantic);
  if (attribute.mSemantic == VertexSemantic::None)
    return value;

  uint offset = (mFixedDesc.mVertexSize * vertexIndex) + attribute.mOffset;
  if (offset + sizeof(T) > mDataSize)
    return value;

  value = *(T*)(mData + offset);
  return value;
}

/// Indices used to define non-sequential primitive construction from vertices,
/// such as shared vertices.
class IndexBuffer : public SafeId32
{
public:
  LightningDeclareType(IndexBuffer, TypeCopyMode::ReferenceType);

  IndexBuffer();
  ~IndexBuffer();

  /// Number of vertex indices currently in buffer.
  /// Can be set manually to invoke vertex shading that number of times, with or
  /// without vertex data.
  uint GetCount();
  void SetCount(uint count);
  uint mIndexCount;

  /// Add a vertex index to the buffer.
  void Add(uint value);

  /// Returns the vertex index that is stored at the given index of this buffer.
  uint Get(uint index);

  /// Clears all stored indices so that new ones can be added.
  void Clear();

  // Internal

  void CopyTo(IndexBuffer& target);

  Array<uint> mData;
  uint mIndexSize;
  bool mGenerated;
};

/// Data that represents a mesh in the way that is intended to be used by
/// graphics hardware.
class Mesh : public Resource
{
public:
  LightningDeclareType(Mesh, TypeCopyMode::ReferenceType);

  /// Makes an anonymous Mesh resource that can be defined by script and
  /// uploaded to the gpu.
  static HandleOf<Mesh> CreateRuntime();
  /// Creates a clone of this Mesh. As a clone is expected to be modified,
  /// an upload function must be called before this mesh can be used.
  HandleOf<Mesh> RuntimeClone();

  Mesh();

  void Unload() override;

  /// Vertex data and attribute semantics for defining data that can be uploaded
  /// to the gpu.
  VertexBuffer mVertices;

  /// Indices used to define non-sequential primitive construction from
  /// vertices, such as shared vertices.
  IndexBuffer mIndices;

  /// The type of primitives to be made with the vertex data.
  PrimitiveType::Enum mPrimitiveType;

  /// Upload vertex buffer and index buffer data to the gpu.
  /// This will also build the aabb and information needed for raycasting.
  void Upload();
  /// Same as Upload except raycasting information will not be built.
  /// This avoids a possible spike when a custom mesh will never need to be
  /// raycasted against.
  void UploadNoRayCastInfo();
  /// Same as Upload except raycasting information and the aabb will not be
  /// built. This should be used when the user is manually setting an aabb or
  /// frustum culling is disabled.
  void UploadNoRayCastInfoOrAabb();

  // Internal

  void UploadInternal(bool updateAabb, bool updateTree);

  uint GetPrimitiveCount();
  uint GetVerticesPerPrimitive();

  template <bool BuildTree>
  void BuildAabbAndTree();
  bool TestRay(GraphicsRayCast& raycast, Mat4 worldTransform);
  bool TestFrustum(const Frustum& frustum);

  template <typename T>
  bool GetPrimitiveData(
      uint primitiveIndex, VertexSemantic::Enum semantic, VertexElementType::Enum type, uint count, T* data);

  MeshRenderData* mRenderData;

  Aabb mAabb;
  Mat4 mBindOffsetInv;
  Array<MeshBone> mBones;
  AvlDynamicAabbTree<uint> mTree;
};

template <typename T>
bool Mesh::GetPrimitiveData(
    uint primitiveIndex, VertexSemantic::Enum semantic, VertexElementType::Enum type, uint count, T* data)
{
  bool result = true;
  uint i0 = 0, i1 = 0, i2 = 0;
  uint verticesPerPrimitive = GetVerticesPerPrimitive();

  uint primitiveCount = mIndices.mIndexCount / verticesPerPrimitive;
  if (primitiveIndex >= primitiveCount)
    return false;

  // If indexing is not coming from buffer
  if (mIndices.mData.Size() == 0)
  {
    i2 = primitiveIndex * verticesPerPrimitive + 2;
    i1 = primitiveIndex * verticesPerPrimitive + 1;
    i0 = primitiveIndex * verticesPerPrimitive + 0;
  }
  else
  {
    switch (mPrimitiveType)
    {
    case Plasma::PrimitiveType::Triangles:
      i2 = mIndices.mData[primitiveIndex * verticesPerPrimitive + 2];
    case Plasma::PrimitiveType::Lines:
      i1 = mIndices.mData[primitiveIndex * verticesPerPrimitive + 1];
    case Plasma::PrimitiveType::Points:
      i0 = mIndices.mData[primitiveIndex * verticesPerPrimitive + 0];
    }
  }

  switch (mPrimitiveType)
  {
  case Plasma::PrimitiveType::Triangles:
    result = result && mVertices.IsValidVertexData(i2, semantic, type, count);
    data[2] = mVertices.GetData<T>(i2, semantic);
  case Plasma::PrimitiveType::Lines:
    result = result && mVertices.IsValidVertexData(i1, semantic, type, count);
    data[1] = mVertices.GetData<T>(i1, semantic);
  case Plasma::PrimitiveType::Points:
    result = result && mVertices.IsValidVertexData(i0, semantic, type, count);
    data[0] = mVertices.GetData<T>(i0, semantic);
  }

  return result;
}

/// Resource Manager for Meshes.
class MeshManager : public ResourceManager
{
public:
  DeclareResourceManager(MeshManager, Mesh);

  MeshManager(BoundType* resourceType);
};

} // namespace Plasma

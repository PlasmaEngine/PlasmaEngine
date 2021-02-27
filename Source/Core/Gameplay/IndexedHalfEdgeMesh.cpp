// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "IndexedHalfEdgeMesh.hpp"

namespace Plasma
{

#define DefineHalfEdgeArrayType(arrayType)                                                                             \
  LightningDefineType(arrayType, builder, type)                                                                            \
  {                                                                                                                    \
    PlasmaBindDocumented();                                                                                              \
                                                                                                                       \
    LightningBindMethod(Get);                                                                                              \
    LightningBindGetter(All);                                                                                              \
    LightningBindGetterProperty(Count);                                                                                    \
  }

DefineHalfEdgeArrayType(IndexedHalfEdgeMeshVertexArray);
DefineHalfEdgeArrayType(IndexedHalfEdgeMeshEdgeArray);
DefineHalfEdgeArrayType(IndexedHalfEdgeFaceEdgeIndexArray);
DefineHalfEdgeArrayType(IndexedHalfEdgeMeshFaceArray);

LightningDefineType(IndexedHalfEdge, builder, type)
{
  LightningBindDefaultCopyDestructor();

  LightningBindFieldGetter(mVertexIndex);
  LightningBindFieldGetter(mTwinIndex);
  LightningBindFieldGetter(mFaceIndex);
}

LightningDefineType(IndexedHalfEdgeFace, builder, type)
{
  LightningBindDefaultCopyDestructor();

  LightningBindGetter(Edges);
}

IndexedHalfEdgeFace::IndexedHalfEdgeFace()
{
  mBoundEdges.mBoundArray = &mEdges;
}

IndexedHalfEdgeFace::BoundEdgeArray* IndexedHalfEdgeFace::GetEdges()
{
  return &mBoundEdges;
}

LightningDefineType(IndexedHalfEdgeMesh, builder, type)
{
  LightningBindDefaultCopyDestructor();

  LightningBindGetter(Vertices);
  LightningBindGetter(Edges);
  LightningBindGetter(Faces);
}

IndexedHalfEdgeMesh::IndexedHalfEdgeMesh()
{
  mBoundVertices.mBoundArray = &mVertices;
  mBoundEdges.mBoundArray = &mEdges;
  mBoundFaces.mBoundArray = &mFaces;
}

void IndexedHalfEdgeMesh::Create(int vertexCount, int edgeCount, int faceCount)
{
  Clear();
  mVertices.Resize(vertexCount);
  mEdges.Resize(edgeCount);
  mFaces.Resize(faceCount);
  for (int i = 0; i < edgeCount; ++i)
    mEdges[i] = new IndexedHalfEdge();
  for (int i = 0; i < faceCount; ++i)
    mFaces[i] = new IndexedHalfEdgeFace();
}

void IndexedHalfEdgeMesh::Clear()
{
  mVertices.Clear();
  DeleteObjectsIn(mEdges);
  DeleteObjectsIn(mFaces);
  mEdges.Clear();
  mFaces.Clear();
}

IndexedHalfEdgeMesh::BoundVertexArray* IndexedHalfEdgeMesh::GetVertices()
{
  return &mBoundVertices;
}

IndexedHalfEdgeMesh::BoundEdgeArray* IndexedHalfEdgeMesh::GetEdges()
{
  return &mBoundEdges;
}

IndexedHalfEdgeMesh::BoundFaceArray* IndexedHalfEdgeMesh::GetFaces()
{
  return &mBoundFaces;
}

} // namespace Plasma

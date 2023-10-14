// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

DeclareEnum3(Generation, None, Build, Import);
DeclareEnum6(GeometryProcessorCodes, NoContent, Success, Failed, LoadGraph, LoadTextures, LoadGraphAndTextures);
DeclareEnum3(ImageProcessorCodes, Success, Failed, Reload);
DeclareEnum3(LoopingMode, Default, Once, Looping);
DeclareEnum2(PhysicsMeshType, PhysicsMesh, ConvexMesh);
DeclareEnum17(MaterialAttribute,
    Unknown,
    DiffuseColor,
    MetallicValue,
    RoughnessValue,
    EmissiveColor,
    SpecularValue,
    TwosidedValue,
    DiffuseMap,
    DiffuseAlphaMap,
    OcclusionMap,
    RoughnessMap,
    MetallicMap,
    OrmMap,
    DisplacementMap,
    NormalMap,
    EmissiveMap,
    SpecularMap);

} // namespace Plasma

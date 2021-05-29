#include "Precompiled.hpp"

#include "SpirVDefinitions.hpp"

namespace Plasma
{

//-------------------------------------------------------------------SpirVInstructions
void SpirVInstructions::LoadSimplifiedDefaults()
{
  mIsSimplifiedInstructions = false;
  mInstructions.Clear();
  AddSimpleInstruction("OpNop", 0);
  AddSimpleInstruction("OpUndef", 1);
  AddSimpleInstruction("OpSourceContinued", 2);
  AddSimpleInstruction("OpSource", 3);
  AddSimpleInstruction("OpSourceExtension", 4);
  AddSimpleInstruction("OpName", 5);
  AddSimpleInstruction("OpMemberName", 6);
  AddSimpleInstruction("OpString", 7);
  AddSimpleInstruction("OpLine", 8);
  AddSimpleInstruction("OpExtension", 10);
  AddSimpleInstruction("OpExtInstImport", 11);
  AddSimpleInstruction("OpExtInst", 12);
  AddSimpleInstruction("OpMemoryModel", 14);
  AddSimpleInstruction("OpEntryPoint", 15);
  AddSimpleInstruction("OpExecutionMode", 16);
  AddSimpleInstruction("OpCapability", 17);
  AddSimpleInstruction("OpTypeVoid", 19);
  AddSimpleInstruction("OpTypeBool", 20);
  AddSimpleInstruction("OpTypeInt", 21);
  AddSimpleInstruction("OpTypeFloat", 22);
  AddSimpleInstruction("OpTypeVector", 23);
  AddSimpleInstruction("OpTypeMatrix", 24);
  AddSimpleInstruction("OpTypeImage", 25);
  AddSimpleInstruction("OpTypeSampler", 26);
  AddSimpleInstruction("OpTypeSampledImage", 27);
  AddSimpleInstruction("OpTypeArray", 28);
  AddSimpleInstruction("OpTypeRuntimeArray", 29);
  AddSimpleInstruction("OpTypeStruct", 30);
  AddSimpleInstruction("OpTypeOpaque", 31);
  AddSimpleInstruction("OpTypePointer", 32);
  AddSimpleInstruction("OpTypeFunction", 33);
  AddSimpleInstruction("OpTypeEvent", 34);
  AddSimpleInstruction("OpTypeDeviceEvent", 35);
  AddSimpleInstruction("OpTypeReserveId", 36);
  AddSimpleInstruction("OpTypeQueue", 37);
  AddSimpleInstruction("OpTypePipe", 38);
  AddSimpleInstruction("OpTypeForwardPointer", 39);
  AddSimpleInstruction("OpConstantTrue", 41);
  AddSimpleInstruction("OpConstantFalse", 42);
  AddSimpleInstruction("OpConstant", 43);
  AddSimpleInstruction("OpConstantComposite", 44);
  AddSimpleInstruction("OpConstantSampler", 45);
  AddSimpleInstruction("OpConstantNull", 46);
  AddSimpleInstruction("OpSpecConstantTrue", 48);
  AddSimpleInstruction("OpSpecConstantFalse", 49);
  AddSimpleInstruction("OpSpecConstant", 50);
  AddSimpleInstruction("OpSpecConstantComposite", 51);
  AddSimpleInstruction("OpSpecConstantOp", 52);
  AddSimpleInstruction("OpFunction", 54);
  AddSimpleInstruction("OpFunctionParameter", 55);
  AddSimpleInstruction("OpFunctionEnd", 56);
  AddSimpleInstruction("OpFunctionCall", 57);
  AddSimpleInstruction("OpVariable", 59);
  AddSimpleInstruction("OpImageTexelPointer", 60);
  AddSimpleInstruction("OpLoad", 61);
  AddSimpleInstruction("OpStore", 62);
  AddSimpleInstruction("OpCopyMemory", 63);
  AddSimpleInstruction("OpCopyMemorySized", 64);
  AddSimpleInstruction("OpAccessChain", 65);
  AddSimpleInstruction("OpInBoundsAccessChain", 66);
  AddSimpleInstruction("OpPtrAccessChain", 67);
  AddSimpleInstruction("OpArrayLength", 68);
  AddSimpleInstruction("OpGenericPtrMemSemantics", 69);
  AddSimpleInstruction("OpInBoundsPtrAccessChain", 70);
  AddSimpleInstruction("OpDecorate", 71);
  AddSimpleInstruction("OpMemberDecorate", 72);
  AddSimpleInstruction("OpDecorationGroup", 73);
  AddSimpleInstruction("OpGroupDecorate", 74);
  AddSimpleInstruction("OpGroupMemberDecorate", 75);
  AddSimpleInstruction("OpVectorExtractDynamic", 77);
  AddSimpleInstruction("OpVectorInsertDynamic", 78);
  AddSimpleInstruction("OpVectorShuffle", 79);
  AddSimpleInstruction("OpCompositeConstruct", 80);
  AddSimpleInstruction("OpCompositeExtract", 81);
  AddSimpleInstruction("OpCompositeInsert", 82);
  AddSimpleInstruction("OpCopyObject", 83);
  AddSimpleInstruction("OpTranspose", 84);
  AddSimpleInstruction("OpSampledImage", 86);
  AddSimpleInstruction("OpImageSampleImplicitLod", 87);
  AddSimpleInstruction("OpImageSampleExplicitLod", 88);
  AddSimpleInstruction("OpImageSampleDrefImplicitLod", 89);
  AddSimpleInstruction("OpImageSampleDrefExplicitLod", 90);
  AddSimpleInstruction("OpImageSampleProjImplicitLod", 91);
  AddSimpleInstruction("OpImageSampleProjExplicitLod", 92);
  AddSimpleInstruction("OpImageSampleProjDrefImplicitLod", 93);
  AddSimpleInstruction("OpImageSampleProjDrefExplicitLod", 94);
  AddSimpleInstruction("OpImageFetch", 95);
  AddSimpleInstruction("OpImageGather", 96);
  AddSimpleInstruction("OpImageDrefGather", 97);
  AddSimpleInstruction("OpImageRead", 98);
  AddSimpleInstruction("OpImageWrite", 99);
  AddSimpleInstruction("OpImage", 100);
  AddSimpleInstruction("OpImageQueryFormat", 101);
  AddSimpleInstruction("OpImageQueryOrder", 102);
  AddSimpleInstruction("OpImageQuerySizeLod", 103);
  AddSimpleInstruction("OpImageQuerySize", 104);
  AddSimpleInstruction("OpImageQueryLod", 105);
  AddSimpleInstruction("OpImageQueryLevels", 106);
  AddSimpleInstruction("OpImageQuerySamples", 107);
  AddSimpleInstruction("OpConvertFToU", 109);
  AddSimpleInstruction("OpConvertFToS", 110);
  AddSimpleInstruction("OpConvertSToF", 111);
  AddSimpleInstruction("OpConvertUToF", 112);
  AddSimpleInstruction("OpUConvert", 113);
  AddSimpleInstruction("OpSConvert", 114);
  AddSimpleInstruction("OpFConvert", 115);
  AddSimpleInstruction("OpQuantizeToF16", 116);
  AddSimpleInstruction("OpConvertPtrToU", 117);
  AddSimpleInstruction("OpSatConvertSToU", 118);
  AddSimpleInstruction("OpSatConvertUToS", 119);
  AddSimpleInstruction("OpConvertUToPtr", 120);
  AddSimpleInstruction("OpPtrCastToGeneric", 121);
  AddSimpleInstruction("OpGenericCastToPtr", 122);
  AddSimpleInstruction("OpGenericCastToPtrExplicit", 123);
  AddSimpleInstruction("OpBitcast", 124);
  AddSimpleInstruction("OpSNegate", 126);
  AddSimpleInstruction("OpFNegate", 127);
  AddSimpleInstruction("OpIAdd", 128);
  AddSimpleInstruction("OpFAdd", 129);
  AddSimpleInstruction("OpISub", 130);
  AddSimpleInstruction("OpFSub", 131);
  AddSimpleInstruction("OpIMul", 132);
  AddSimpleInstruction("OpFMul", 133);
  AddSimpleInstruction("OpUDiv", 134);
  AddSimpleInstruction("OpSDiv", 135);
  AddSimpleInstruction("OpFDiv", 136);
  AddSimpleInstruction("OpUMod", 137);
  AddSimpleInstruction("OpSRem", 138);
  AddSimpleInstruction("OpSMod", 139);
  AddSimpleInstruction("OpFRem", 140);
  AddSimpleInstruction("OpFMod", 141);
  AddSimpleInstruction("OpVectorTimesScalar", 142);
  AddSimpleInstruction("OpMatrixTimesScalar", 143);
  AddSimpleInstruction("OpVectorTimesMatrix", 144);
  AddSimpleInstruction("OpMatrixTimesVector", 145);
  AddSimpleInstruction("OpMatrixTimesMatrix", 146);
  AddSimpleInstruction("OpOuterProduct", 147);
  AddSimpleInstruction("OpDot", 148);
  AddSimpleInstruction("OpIAddCarry", 149);
  AddSimpleInstruction("OpISubBorrow", 150);
  AddSimpleInstruction("OpUMulExtended", 151);
  AddSimpleInstruction("OpSMulExtended", 152);
  AddSimpleInstruction("OpAny", 154);
  AddSimpleInstruction("OpAll", 155);
  AddSimpleInstruction("OpIsNan", 156);
  AddSimpleInstruction("OpIsInf", 157);
  AddSimpleInstruction("OpIsFinite", 158);
  AddSimpleInstruction("OpIsNormal", 159);
  AddSimpleInstruction("OpSignBitSet", 160);
  AddSimpleInstruction("OpLessOrGreater", 161);
  AddSimpleInstruction("OpOrdered", 162);
  AddSimpleInstruction("OpUnordered", 163);
  AddSimpleInstruction("OpLogicalEqual", 164);
  AddSimpleInstruction("OpLogicalNotEqual", 165);
  AddSimpleInstruction("OpLogicalOr", 166);
  AddSimpleInstruction("OpLogicalAnd", 167);
  AddSimpleInstruction("OpLogicalNot", 168);
  AddSimpleInstruction("OpSelect", 169);
  AddSimpleInstruction("OpIEqual", 170);
  AddSimpleInstruction("OpINotEqual", 171);
  AddSimpleInstruction("OpUGreaterThan", 172);
  AddSimpleInstruction("OpSGreaterThan", 173);
  AddSimpleInstruction("OpUGreaterThanEqual", 174);
  AddSimpleInstruction("OpSGreaterThanEqual", 175);
  AddSimpleInstruction("OpULessThan", 176);
  AddSimpleInstruction("OpSLessThan", 177);
  AddSimpleInstruction("OpULessThanEqual", 178);
  AddSimpleInstruction("OpSLessThanEqual", 179);
  AddSimpleInstruction("OpFOrdEqual", 180);
  AddSimpleInstruction("OpFUnordEqual", 181);
  AddSimpleInstruction("OpFOrdNotEqual", 182);
  AddSimpleInstruction("OpFUnordNotEqual", 183);
  AddSimpleInstruction("OpFOrdLessThan", 184);
  AddSimpleInstruction("OpFUnordLessThan", 185);
  AddSimpleInstruction("OpFOrdGreaterThan", 186);
  AddSimpleInstruction("OpFUnordGreaterThan", 187);
  AddSimpleInstruction("OpFOrdLessThanEqual", 188);
  AddSimpleInstruction("OpFUnordLessThanEqual", 189);
  AddSimpleInstruction("OpFOrdGreaterThanEqual", 190);
  AddSimpleInstruction("OpFUnordGreaterThanEqual", 191);
  AddSimpleInstruction("OpShiftRightLogical", 194);
  AddSimpleInstruction("OpShiftRightArithmetic", 195);
  AddSimpleInstruction("OpShiftLeftLogical", 196);
  AddSimpleInstruction("OpBitwiseOr", 197);
  AddSimpleInstruction("OpBitwiseXor", 198);
  AddSimpleInstruction("OpBitwiseAnd", 199);
  AddSimpleInstruction("OpNot", 200);
  AddSimpleInstruction("OpBitFieldInsert", 201);
  AddSimpleInstruction("OpBitFieldSExtract", 202);
  AddSimpleInstruction("OpBitFieldUExtract", 203);
  AddSimpleInstruction("OpBitReverse", 204);
  AddSimpleInstruction("OpBitCount", 205);
  AddSimpleInstruction("OpDPdx", 207);
  AddSimpleInstruction("OpDPdy", 208);
  AddSimpleInstruction("OpFwidth", 209);
  AddSimpleInstruction("OpDPdxFine", 210);
  AddSimpleInstruction("OpDPdyFine", 211);
  AddSimpleInstruction("OpFwidthFine", 212);
  AddSimpleInstruction("OpDPdxCoarse", 213);
  AddSimpleInstruction("OpDPdyCoarse", 214);
  AddSimpleInstruction("OpFwidthCoarse", 215);
  AddSimpleInstruction("OpEmitVertex", 218);
  AddSimpleInstruction("OpEndPrimitive", 219);
  AddSimpleInstruction("OpEmitStreamVertex", 220);
  AddSimpleInstruction("OpEndStreamPrimitive", 221);
  AddSimpleInstruction("OpControlBarrier", 224);
  AddSimpleInstruction("OpMemoryBarrier", 225);
  AddSimpleInstruction("OpAtomicLoad", 227);
  AddSimpleInstruction("OpAtomicStore", 228);
  AddSimpleInstruction("OpAtomicExchange", 229);
  AddSimpleInstruction("OpAtomicCompareExchange", 230);
  AddSimpleInstruction("OpAtomicCompareExchangeWeak", 231);
  AddSimpleInstruction("OpAtomicIIncrement", 232);
  AddSimpleInstruction("OpAtomicIDecrement", 233);
  AddSimpleInstruction("OpAtomicIAdd", 234);
  AddSimpleInstruction("OpAtomicISub", 235);
  AddSimpleInstruction("OpAtomicSMin", 236);
  AddSimpleInstruction("OpAtomicUMin", 237);
  AddSimpleInstruction("OpAtomicSMax", 238);
  AddSimpleInstruction("OpAtomicUMax", 239);
  AddSimpleInstruction("OpAtomicAnd", 240);
  AddSimpleInstruction("OpAtomicOr", 241);
  AddSimpleInstruction("OpAtomicXor", 242);
  AddSimpleInstruction("OpPhi", 245);
  AddSimpleInstruction("OpLoopMerge", 246);
  AddSimpleInstruction("OpSelectionMerge", 247);
  AddSimpleInstruction("OpLabel", 248);
  AddSimpleInstruction("OpBranch", 249);
  AddSimpleInstruction("OpBranchConditional", 250);
  AddSimpleInstruction("OpSwitch", 251);
  AddSimpleInstruction("OpKill", 252);
  AddSimpleInstruction("OpReturn", 253);
  AddSimpleInstruction("OpReturnValue", 254);
  AddSimpleInstruction("OpUnreachable", 255);
  AddSimpleInstruction("OpLifetimeStart", 256);
  AddSimpleInstruction("OpLifetimeStop", 257);
  AddSimpleInstruction("OpGroupAsyncCopy", 259);
  AddSimpleInstruction("OpGroupWaitEvents", 260);
  AddSimpleInstruction("OpGroupAll", 261);
  AddSimpleInstruction("OpGroupAny", 262);
  AddSimpleInstruction("OpGroupBroadcast", 263);
  AddSimpleInstruction("OpGroupIAdd", 264);
  AddSimpleInstruction("OpGroupFAdd", 265);
  AddSimpleInstruction("OpGroupFMin", 266);
  AddSimpleInstruction("OpGroupUMin", 267);
  AddSimpleInstruction("OpGroupSMin", 268);
  AddSimpleInstruction("OpGroupFMax", 269);
  AddSimpleInstruction("OpGroupUMax", 270);
  AddSimpleInstruction("OpGroupSMax", 271);
  AddSimpleInstruction("OpReadPipe", 274);
  AddSimpleInstruction("OpWritePipe", 275);
  AddSimpleInstruction("OpReservedReadPipe", 276);
  AddSimpleInstruction("OpReservedWritePipe", 277);
  AddSimpleInstruction("OpReserveReadPipePackets", 278);
  AddSimpleInstruction("OpReserveWritePipePackets", 279);
  AddSimpleInstruction("OpCommitReadPipe", 280);
  AddSimpleInstruction("OpCommitWritePipe", 281);
  AddSimpleInstruction("OpIsValidReserveId", 282);
  AddSimpleInstruction("OpGetNumPipePackets", 283);
  AddSimpleInstruction("OpGetMaxPipePackets", 284);
  AddSimpleInstruction("OpGroupReserveReadPipePackets", 285);
  AddSimpleInstruction("OpGroupReserveWritePipePackets", 286);
  AddSimpleInstruction("OpGroupCommitReadPipe", 287);
  AddSimpleInstruction("OpGroupCommitWritePipe", 288);
  AddSimpleInstruction("OpEnqueueMarker", 291);
  AddSimpleInstruction("OpEnqueueKernel", 292);
  AddSimpleInstruction("OpGetKernelNDrangeSubGroupCount", 293);
  AddSimpleInstruction("OpGetKernelNDrangeMaxSubGroupSize", 294);
  AddSimpleInstruction("OpGetKernelWorkGroupSize", 295);
  AddSimpleInstruction("OpGetKernelPreferredWorkGroupSizeMultiple", 296);
  AddSimpleInstruction("OpRetainEvent", 297);
  AddSimpleInstruction("OpReleaseEvent", 298);
  AddSimpleInstruction("OpCreateUserEvent", 299);
  AddSimpleInstruction("OpIsValidEvent", 300);
  AddSimpleInstruction("OpSetUserEventStatus", 301);
  AddSimpleInstruction("OpCaptureEventProfilingInfo", 302);
  AddSimpleInstruction("OpGetDefaultQueue", 303);
  AddSimpleInstruction("OpBuildNDRange", 304);
  AddSimpleInstruction("OpImageSparseSampleImplicitLod", 305);
  AddSimpleInstruction("OpImageSparseSampleExplicitLod", 306);
  AddSimpleInstruction("OpImageSparseSampleDrefImplicitLod", 307);
  AddSimpleInstruction("OpImageSparseSampleDrefExplicitLod", 308);
  AddSimpleInstruction("OpImageSparseSampleProjImplicitLod", 309);
  AddSimpleInstruction("OpImageSparseSampleProjExplicitLod", 310);
  AddSimpleInstruction("OpImageSparseSampleProjDrefImplicitLod", 311);
  AddSimpleInstruction("OpImageSparseSampleProjDrefExplicitLod", 312);
  AddSimpleInstruction("OpImageSparseFetch", 313);
  AddSimpleInstruction("OpImageSparseGather", 314);
  AddSimpleInstruction("OpImageSparseDrefGather", 315);
  AddSimpleInstruction("OpImageSparseTexelsResident", 316);
  AddSimpleInstruction("OpNoLine", 317);
  AddSimpleInstruction("OpAtomicFlagTestAndSet", 318);
  AddSimpleInstruction("OpAtomicFlagClear", 319);
  AddSimpleInstruction("OpImageSparseRead", 320);
  AddSimpleInstruction("OpSizeOf", 321);
  AddSimpleInstruction("OpTypePipeStorage", 322);
  AddSimpleInstruction("OpConstantPipeStorage", 323);
  AddSimpleInstruction("OpCreatePipeFromPipeStorage", 324);
  AddSimpleInstruction("OpGetKernelLocalSizeForSubgroupCount", 325);
  AddSimpleInstruction("OpGetKernelMaxNumSubgroups", 326);
  AddSimpleInstruction("OpTypeNamedBarrier", 327);
  AddSimpleInstruction("OpNamedBarrierInitialize", 328);
  AddSimpleInstruction("OpMemoryNamedBarrier", 329);
  AddSimpleInstruction("OpModuleProcessed", 330);
  AddSimpleInstruction("OpExecutionModeId", 331);
  AddSimpleInstruction("OpDecorateId", 332);
  AddSimpleInstruction("OpGroupNonUniformElect", 333);
  AddSimpleInstruction("OpGroupNonUniformAll", 334);
  AddSimpleInstruction("OpGroupNonUniformAny", 335);
  AddSimpleInstruction("OpGroupNonUniformAllEqual", 336);
  AddSimpleInstruction("OpGroupNonUniformBroadcast", 337);
  AddSimpleInstruction("OpGroupNonUniformBroadcastFirst", 338);
  AddSimpleInstruction("OpGroupNonUniformBallot", 339);
  AddSimpleInstruction("OpGroupNonUniformInverseBallot", 340);
  AddSimpleInstruction("OpGroupNonUniformBallotBitExtract", 341);
  AddSimpleInstruction("OpGroupNonUniformBallotBitCount", 342);
  AddSimpleInstruction("OpGroupNonUniformBallotFindLSB", 343);
  AddSimpleInstruction("OpGroupNonUniformBallotFindMSB", 344);
  AddSimpleInstruction("OpGroupNonUniformShuffle", 345);
  AddSimpleInstruction("OpGroupNonUniformShuffleXor", 346);
  AddSimpleInstruction("OpGroupNonUniformShuffleUp", 347);
  AddSimpleInstruction("OpGroupNonUniformShuffleDown", 348);
  AddSimpleInstruction("OpGroupNonUniformIAdd", 349);
  AddSimpleInstruction("OpGroupNonUniformFAdd", 350);
  AddSimpleInstruction("OpGroupNonUniformIMul", 351);
  AddSimpleInstruction("OpGroupNonUniformFMul", 352);
  AddSimpleInstruction("OpGroupNonUniformSMin", 353);
  AddSimpleInstruction("OpGroupNonUniformUMin", 354);
  AddSimpleInstruction("OpGroupNonUniformFMin", 355);
  AddSimpleInstruction("OpGroupNonUniformSMax", 356);
  AddSimpleInstruction("OpGroupNonUniformUMax", 357);
  AddSimpleInstruction("OpGroupNonUniformFMax", 358);
  AddSimpleInstruction("OpGroupNonUniformBitwiseAnd", 359);
  AddSimpleInstruction("OpGroupNonUniformBitwiseOr", 360);
  AddSimpleInstruction("OpGroupNonUniformBitwiseXor", 361);
  AddSimpleInstruction("OpGroupNonUniformLogicalAnd", 362);
  AddSimpleInstruction("OpGroupNonUniformLogicalOr", 363);
  AddSimpleInstruction("OpGroupNonUniformLogicalXor", 364);
  AddSimpleInstruction("OpGroupNonUniformQuadBroadcast", 365);
  AddSimpleInstruction("OpGroupNonUniformQuadSwap", 366);
  AddSimpleInstruction("OpCopyLogical", 400);
  AddSimpleInstruction("OpPtrEqual", 401);
  AddSimpleInstruction("OpPtrNotEqual", 402);
  AddSimpleInstruction("OpPtrDiff", 403);
  AddSimpleInstruction("OpSubgroupBallotKHR", 4421);
  AddSimpleInstruction("OpSubgroupFirstInvocationKHR", 4422);
  AddSimpleInstruction("OpSubgroupAllKHR", 4428);
  AddSimpleInstruction("OpSubgroupAnyKHR", 4429);
  AddSimpleInstruction("OpSubgroupAllEqualKHR", 4430);
  AddSimpleInstruction("OpSubgroupReadInvocationKHR", 4432);
  AddSimpleInstruction("OpGroupIAddNonUniformAMD", 5000);
  AddSimpleInstruction("OpGroupFAddNonUniformAMD", 5001);
  AddSimpleInstruction("OpGroupFMinNonUniformAMD", 5002);
  AddSimpleInstruction("OpGroupUMinNonUniformAMD", 5003);
  AddSimpleInstruction("OpGroupSMinNonUniformAMD", 5004);
  AddSimpleInstruction("OpGroupFMaxNonUniformAMD", 5005);
  AddSimpleInstruction("OpGroupUMaxNonUniformAMD", 5006);
  AddSimpleInstruction("OpGroupSMaxNonUniformAMD", 5007);
  AddSimpleInstruction("OpFragmentMaskFetchAMD", 5011);
  AddSimpleInstruction("OpFragmentFetchAMD", 5012);
  AddSimpleInstruction("OpReadClockKHR", 5056);
  AddSimpleInstruction("OpImageSampleFootprintNV", 5283);
  AddSimpleInstruction("OpGroupNonUniformPartitionNV", 5296);
  AddSimpleInstruction("OpWritePackedPrimitiveIndices4x8NV", 5299);
  AddSimpleInstruction("OpReportIntersectionNV", 5334);
  AddSimpleInstruction("OpIgnoreIntersectionNV", 5335);
  AddSimpleInstruction("OpTerminateRayNV", 5336);
  AddSimpleInstruction("OpTraceNV", 5337);
  AddSimpleInstruction("OpTypeAccelerationStructureNV", 5341);
  AddSimpleInstruction("OpExecuteCallableNV", 5344);
  AddSimpleInstruction("OpTypeCooperativeMatrixNV", 5358);
  AddSimpleInstruction("OpCooperativeMatrixLoadNV", 5359);
  AddSimpleInstruction("OpCooperativeMatrixStoreNV", 5360);
  AddSimpleInstruction("OpCooperativeMatrixMulAddNV", 5361);
  AddSimpleInstruction("OpCooperativeMatrixLengthNV", 5362);
  AddSimpleInstruction("OpBeginInvocationInterlockEXT", 5364);
  AddSimpleInstruction("OpEndInvocationInterlockEXT", 5365);
  AddSimpleInstruction("OpDemoteToHelperInvocationEXT", 5380);
  AddSimpleInstruction("OpIsHelperInvocationEXT", 5381);
  AddSimpleInstruction("OpSubgroupShuffleINTEL", 5571);
  AddSimpleInstruction("OpSubgroupShuffleDownINTEL", 5572);
  AddSimpleInstruction("OpSubgroupShuffleUpINTEL", 5573);
  AddSimpleInstruction("OpSubgroupShuffleXorINTEL", 5574);
  AddSimpleInstruction("OpSubgroupBlockReadINTEL", 5575);
  AddSimpleInstruction("OpSubgroupBlockWriteINTEL", 5576);
  AddSimpleInstruction("OpSubgroupImageBlockReadINTEL", 5577);
  AddSimpleInstruction("OpSubgroupImageBlockWriteINTEL", 5578);
  AddSimpleInstruction("OpSubgroupImageMediaBlockReadINTEL", 5580);
  AddSimpleInstruction("OpSubgroupImageMediaBlockWriteINTEL", 5581);
  AddSimpleInstruction("OpUCountLeadingZerosINTEL", 5585);
  AddSimpleInstruction("OpUCountTrailingZerosINTEL", 5586);
  AddSimpleInstruction("OpAbsISubINTEL", 5587);
  AddSimpleInstruction("OpAbsUSubINTEL", 5588);
  AddSimpleInstruction("OpIAddSatINTEL", 5589);
  AddSimpleInstruction("OpUAddSatINTEL", 5590);
  AddSimpleInstruction("OpIAverageINTEL", 5591);
  AddSimpleInstruction("OpUAverageINTEL", 5592);
  AddSimpleInstruction("OpIAverageRoundedINTEL", 5593);
  AddSimpleInstruction("OpUAverageRoundedINTEL", 5594);
  AddSimpleInstruction("OpISubSatINTEL", 5595);
  AddSimpleInstruction("OpUSubSatINTEL", 5596);
  AddSimpleInstruction("OpIMul32x16INTEL", 5597);
  AddSimpleInstruction("OpUMul32x16INTEL", 5598);
  AddSimpleInstruction("OpDecorateString", 5632);
  AddSimpleInstruction("OpDecorateStringGOOGLE", 5632);
  AddSimpleInstruction("OpMemberDecorateString", 5633);
  AddSimpleInstruction("OpMemberDecorateStringGOOGLE", 5633);
  AddSimpleInstruction("OpVmeImageINTEL", 5699);
  AddSimpleInstruction("OpTypeVmeImageINTEL", 5700);
  AddSimpleInstruction("OpTypeAvcImePayloadINTEL", 5701);
  AddSimpleInstruction("OpTypeAvcRefPayloadINTEL", 5702);
  AddSimpleInstruction("OpTypeAvcSicPayloadINTEL", 5703);
  AddSimpleInstruction("OpTypeAvcMcePayloadINTEL", 5704);
  AddSimpleInstruction("OpTypeAvcMceResultINTEL", 5705);
  AddSimpleInstruction("OpTypeAvcImeResultINTEL", 5706);
  AddSimpleInstruction("OpTypeAvcImeResultSingleReferenceStreamoutINTEL", 5707);
  AddSimpleInstruction("OpTypeAvcImeResultDualReferenceStreamoutINTEL", 5708);
  AddSimpleInstruction("OpTypeAvcImeSingleReferenceStreaminINTEL", 5709);
  AddSimpleInstruction("OpTypeAvcImeDualReferenceStreaminINTEL", 5710);
  AddSimpleInstruction("OpTypeAvcRefResultINTEL", 5711);
  AddSimpleInstruction("OpTypeAvcSicResultINTEL", 5712);
  AddSimpleInstruction("OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL", 5713);
  AddSimpleInstruction("OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL", 5714);
  AddSimpleInstruction("OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL", 5715);
  AddSimpleInstruction("OpSubgroupAvcMceSetInterShapePenaltyINTEL", 5716);
  AddSimpleInstruction("OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL", 5717);
  AddSimpleInstruction("OpSubgroupAvcMceSetInterDirectionPenaltyINTEL", 5718);
  AddSimpleInstruction("OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL", 5719);
  AddSimpleInstruction("OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL", 5720);
  AddSimpleInstruction("OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL", 5721);
  AddSimpleInstruction("OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL", 5722);
  AddSimpleInstruction("OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL", 5723);
  AddSimpleInstruction("OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL", 5724);
  AddSimpleInstruction("OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL", 5725);
  AddSimpleInstruction("OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL", 5726);
  AddSimpleInstruction("OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL", 5727);
  AddSimpleInstruction("OpSubgroupAvcMceSetAcOnlyHaarINTEL", 5728);
  AddSimpleInstruction("OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL", 5729);
  AddSimpleInstruction("OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL", 5730);
  AddSimpleInstruction("OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL", 5731);
  AddSimpleInstruction("OpSubgroupAvcMceConvertToImePayloadINTEL", 5732);
  AddSimpleInstruction("OpSubgroupAvcMceConvertToImeResultINTEL", 5733);
  AddSimpleInstruction("OpSubgroupAvcMceConvertToRefPayloadINTEL", 5734);
  AddSimpleInstruction("OpSubgroupAvcMceConvertToRefResultINTEL", 5735);
  AddSimpleInstruction("OpSubgroupAvcMceConvertToSicPayloadINTEL", 5736);
  AddSimpleInstruction("OpSubgroupAvcMceConvertToSicResultINTEL", 5737);
  AddSimpleInstruction("OpSubgroupAvcMceGetMotionVectorsINTEL", 5738);
  AddSimpleInstruction("OpSubgroupAvcMceGetInterDistortionsINTEL", 5739);
  AddSimpleInstruction("OpSubgroupAvcMceGetBestInterDistortionsINTEL", 5740);
  AddSimpleInstruction("OpSubgroupAvcMceGetInterMajorShapeINTEL", 5741);
  AddSimpleInstruction("OpSubgroupAvcMceGetInterMinorShapeINTEL", 5742);
  AddSimpleInstruction("OpSubgroupAvcMceGetInterDirectionsINTEL", 5743);
  AddSimpleInstruction("OpSubgroupAvcMceGetInterMotionVectorCountINTEL", 5744);
  AddSimpleInstruction("OpSubgroupAvcMceGetInterReferenceIdsINTEL", 5745);
  AddSimpleInstruction("OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL", 5746);
  AddSimpleInstruction("OpSubgroupAvcImeInitializeINTEL", 5747);
  AddSimpleInstruction("OpSubgroupAvcImeSetSingleReferenceINTEL", 5748);
  AddSimpleInstruction("OpSubgroupAvcImeSetDualReferenceINTEL", 5749);
  AddSimpleInstruction("OpSubgroupAvcImeRefWindowSizeINTEL", 5750);
  AddSimpleInstruction("OpSubgroupAvcImeAdjustRefOffsetINTEL", 5751);
  AddSimpleInstruction("OpSubgroupAvcImeConvertToMcePayloadINTEL", 5752);
  AddSimpleInstruction("OpSubgroupAvcImeSetMaxMotionVectorCountINTEL", 5753);
  AddSimpleInstruction("OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL", 5754);
  AddSimpleInstruction("OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL", 5755);
  AddSimpleInstruction("OpSubgroupAvcImeSetWeightedSadINTEL", 5756);
  AddSimpleInstruction("OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL", 5757);
  AddSimpleInstruction("OpSubgroupAvcImeEvaluateWithDualReferenceINTEL", 5758);
  AddSimpleInstruction("OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL", 5759);
  AddSimpleInstruction("OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL", 5760);
  AddSimpleInstruction("OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL", 5761);
  AddSimpleInstruction("OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL", 5762);
  AddSimpleInstruction("OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL", 5763);
  AddSimpleInstruction("OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL", 5764);
  AddSimpleInstruction("OpSubgroupAvcImeConvertToMceResultINTEL", 5765);
  AddSimpleInstruction("OpSubgroupAvcImeGetSingleReferenceStreaminINTEL", 5766);
  AddSimpleInstruction("OpSubgroupAvcImeGetDualReferenceStreaminINTEL", 5767);
  AddSimpleInstruction("OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL", 5768);
  AddSimpleInstruction("OpSubgroupAvcImeStripDualReferenceStreamoutINTEL", 5769);
  AddSimpleInstruction("OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL", 5770);
  AddSimpleInstruction("OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL", 5771);
  AddSimpleInstruction("OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL", 5772);
  AddSimpleInstruction("OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL", 5773);
  AddSimpleInstruction("OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL", 5774);
  AddSimpleInstruction("OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL", 5775);
  AddSimpleInstruction("OpSubgroupAvcImeGetBorderReachedINTEL", 5776);
  AddSimpleInstruction("OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL", 5777);
  AddSimpleInstruction("OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL", 5778);
  AddSimpleInstruction("OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL", 5779);
  AddSimpleInstruction("OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL", 5780);
  AddSimpleInstruction("OpSubgroupAvcFmeInitializeINTEL", 5781);
  AddSimpleInstruction("OpSubgroupAvcBmeInitializeINTEL", 5782);
  AddSimpleInstruction("OpSubgroupAvcRefConvertToMcePayloadINTEL", 5783);
  AddSimpleInstruction("OpSubgroupAvcRefSetBidirectionalMixDisableINTEL", 5784);
  AddSimpleInstruction("OpSubgroupAvcRefSetBilinearFilterEnableINTEL", 5785);
  AddSimpleInstruction("OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL", 5786);
  AddSimpleInstruction("OpSubgroupAvcRefEvaluateWithDualReferenceINTEL", 5787);
  AddSimpleInstruction("OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL", 5788);
  AddSimpleInstruction("OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL", 5789);
  AddSimpleInstruction("OpSubgroupAvcRefConvertToMceResultINTEL", 5790);
  AddSimpleInstruction("OpSubgroupAvcSicInitializeINTEL", 5791);
  AddSimpleInstruction("OpSubgroupAvcSicConfigureSkcINTEL", 5792);
  AddSimpleInstruction("OpSubgroupAvcSicConfigureIpeLumaINTEL", 5793);
  AddSimpleInstruction("OpSubgroupAvcSicConfigureIpeLumaChromaINTEL", 5794);
  AddSimpleInstruction("OpSubgroupAvcSicGetMotionVectorMaskINTEL", 5795);
  AddSimpleInstruction("OpSubgroupAvcSicConvertToMcePayloadINTEL", 5796);
  AddSimpleInstruction("OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL", 5797);
  AddSimpleInstruction("OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL", 5798);
  AddSimpleInstruction("OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL", 5799);
  AddSimpleInstruction("OpSubgroupAvcSicSetBilinearFilterEnableINTEL", 5800);
  AddSimpleInstruction("OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL", 5801);
  AddSimpleInstruction("OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL", 5802);
  AddSimpleInstruction("OpSubgroupAvcSicEvaluateIpeINTEL", 5803);
  AddSimpleInstruction("OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL", 5804);
  AddSimpleInstruction("OpSubgroupAvcSicEvaluateWithDualReferenceINTEL", 5805);
  AddSimpleInstruction("OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL", 5806);
  AddSimpleInstruction("OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL", 5807);
  AddSimpleInstruction("OpSubgroupAvcSicConvertToMceResultINTEL", 5808);
  AddSimpleInstruction("OpSubgroupAvcSicGetIpeLumaShapeINTEL", 5809);
  AddSimpleInstruction("OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL", 5810);
  AddSimpleInstruction("OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL", 5811);
  AddSimpleInstruction("OpSubgroupAvcSicGetPackedIpeLumaModesINTEL", 5812);
  AddSimpleInstruction("OpSubgroupAvcSicGetIpeChromaModeINTEL", 5813);
  AddSimpleInstruction("OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL", 5814);
  AddSimpleInstruction("OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL", 5815);
  AddSimpleInstruction("OpSubgroupAvcSicGetInterRawSadsINTEL", 5816);
  AddSimpleInstruction("OpMax", 0x7fffffff);
}

#include "GLSL.std.450.h"

void SpirVInstructions::LoadSimplifiedGlsl450Defaults()
{
  AddSimpleInstruction("Bad", GLSLstd450Bad);
  AddSimpleInstruction("Round", GLSLstd450Round);
  AddSimpleInstruction("RoundEven", GLSLstd450RoundEven);
  AddSimpleInstruction("Trunc", GLSLstd450Trunc);
  AddSimpleInstruction("FAbs", GLSLstd450FAbs);
  AddSimpleInstruction("SAbs", GLSLstd450SAbs);
  AddSimpleInstruction("FSign", GLSLstd450FSign);
  AddSimpleInstruction("SSign", GLSLstd450SSign);
  AddSimpleInstruction("Floor", GLSLstd450Floor);
  AddSimpleInstruction("Ceil", GLSLstd450Ceil);
  AddSimpleInstruction("Fract", GLSLstd450Fract);
  AddSimpleInstruction("Radians", GLSLstd450Radians);
  AddSimpleInstruction("Degrees", GLSLstd450Degrees);
  AddSimpleInstruction("Sin", GLSLstd450Sin);
  AddSimpleInstruction("Cos", GLSLstd450Cos);
  AddSimpleInstruction("Tan", GLSLstd450Tan);
  AddSimpleInstruction("Asin", GLSLstd450Asin);
  AddSimpleInstruction("Acos", GLSLstd450Acos);
  AddSimpleInstruction("Atan", GLSLstd450Atan);
  AddSimpleInstruction("Sinh", GLSLstd450Sinh);
  AddSimpleInstruction("Cosh", GLSLstd450Cosh);
  AddSimpleInstruction("Tanh", GLSLstd450Tanh);
  AddSimpleInstruction("Asinh", GLSLstd450Asinh);
  AddSimpleInstruction("Acosh", GLSLstd450Acosh);
  AddSimpleInstruction("Atanh", GLSLstd450Atanh);
  AddSimpleInstruction("Atan2", GLSLstd450Atan2);
  AddSimpleInstruction("Pow", GLSLstd450Pow);
  AddSimpleInstruction("Exp", GLSLstd450Exp);
  AddSimpleInstruction("Log", GLSLstd450Log);
  AddSimpleInstruction("Exp2", GLSLstd450Exp2);
  AddSimpleInstruction("Log2", GLSLstd450Log2);
  AddSimpleInstruction("Sqrt", GLSLstd450Sqrt);
  AddSimpleInstruction("InverseSqrt", GLSLstd450InverseSqrt);
  AddSimpleInstruction("Determinant", GLSLstd450Determinant);
  AddSimpleInstruction("MatrixInverse", GLSLstd450MatrixInverse);
  AddSimpleInstruction("Modf", GLSLstd450Modf);
  AddSimpleInstruction("ModfStruct", GLSLstd450ModfStruct);
  AddSimpleInstruction("FMin", GLSLstd450FMin);
  AddSimpleInstruction("UMin", GLSLstd450UMin);
  AddSimpleInstruction("SMin", GLSLstd450SMin);
  AddSimpleInstruction("FMax", GLSLstd450FMax);
  AddSimpleInstruction("UMax", GLSLstd450UMax);
  AddSimpleInstruction("SMax", GLSLstd450SMax);
  AddSimpleInstruction("FClamp", GLSLstd450FClamp);
  AddSimpleInstruction("UClamp", GLSLstd450UClamp);
  AddSimpleInstruction("SClamp", GLSLstd450SClamp);
  AddSimpleInstruction("FMix", GLSLstd450FMix);
  AddSimpleInstruction("IMix", GLSLstd450IMix);
  AddSimpleInstruction("Step", GLSLstd450Step);
  AddSimpleInstruction("SmoothStep", GLSLstd450SmoothStep);
  AddSimpleInstruction("Fma", GLSLstd450Fma);
  AddSimpleInstruction("Frexp", GLSLstd450Frexp);
  AddSimpleInstruction("FrexpStruct", GLSLstd450FrexpStruct);
  AddSimpleInstruction("Ldexp", GLSLstd450Ldexp);
  AddSimpleInstruction("PackSnorm4x8", GLSLstd450PackSnorm4x8);
  AddSimpleInstruction("PackUnorm4x8", GLSLstd450PackUnorm4x8);
  AddSimpleInstruction("PackSnorm2x16", GLSLstd450PackSnorm2x16);
  AddSimpleInstruction("PackUnorm2x16", GLSLstd450PackUnorm2x16);
  AddSimpleInstruction("PackHalf2x16", GLSLstd450PackHalf2x16);
  AddSimpleInstruction("PackDouble2x32", GLSLstd450PackDouble2x32);
  AddSimpleInstruction("UnpackSnorm2x16", GLSLstd450UnpackSnorm2x16);
  AddSimpleInstruction("UnpackUnorm2x16", GLSLstd450UnpackUnorm2x16);
  AddSimpleInstruction("UnpackHalf2x16", GLSLstd450UnpackHalf2x16);
  AddSimpleInstruction("UnpackSnorm4x8", GLSLstd450UnpackSnorm4x8);
  AddSimpleInstruction("UnpackUnorm4x8", GLSLstd450UnpackUnorm4x8);
  AddSimpleInstruction("UnpackDouble2x32", GLSLstd450UnpackDouble2x32);
  AddSimpleInstruction("Length", GLSLstd450Length);
  AddSimpleInstruction("Distance", GLSLstd450Distance);
  AddSimpleInstruction("Cross", GLSLstd450Cross);
  AddSimpleInstruction("Normalize", GLSLstd450Normalize);
  AddSimpleInstruction("FaceForward", GLSLstd450FaceForward);
  AddSimpleInstruction("Reflect", GLSLstd450Reflect);
  AddSimpleInstruction("Refract", GLSLstd450Refract);
  AddSimpleInstruction("FindILsb", GLSLstd450FindILsb);
  AddSimpleInstruction("FindSMsb", GLSLstd450FindSMsb);
  AddSimpleInstruction("FindUMsb", GLSLstd450FindUMsb);
  AddSimpleInstruction("InterpolateAtCentroid", GLSLstd450InterpolateAtCentroid);
  AddSimpleInstruction("InterpolateAtSample", GLSLstd450InterpolateAtSample);
  AddSimpleInstruction("InterpolateAtOffset", GLSLstd450InterpolateAtOffset);
  AddSimpleInstruction("NMin", GLSLstd450NMin);
  AddSimpleInstruction("NMax", GLSLstd450NMax);
  AddSimpleInstruction("NClamp", GLSLstd450NClamp);
  AddSimpleInstruction("Count", GLSLstd450Count);
}

void SpirVInstructions::Load(Lightning::JsonValue* jsonRoot)
{
  if(jsonRoot == nullptr)
    return;

  mIsSimplifiedInstructions = false;
  mInstructions.Clear();
  Lightning::JsonValue* instructionsGrammar = jsonRoot->GetMember("instructions", Lightning::JsonErrorMode::DefaultValue);
  ParseInstructions(instructionsGrammar, mInstructions);
}

bool SpirVInstructions::IsSimplifiedInstructions() const
{
  return mIsSimplifiedInstructions;
}

const SpirVInstructions::Instruction* SpirVInstructions::FindInstruction(StringParam opName) const
{
  return mInstructions.FindPointer(opName);
}

void SpirVInstructions::AddSimpleInstruction(StringParam opName, int opCode)
{
  Instruction& instruction = mInstructions[opName];
  instruction.mName = opName;
  instruction.mOpCode = opCode;
}

void SpirVInstructions::ParseInstructions(Lightning::JsonValue* instructionsJson, HashMap<String, Instruction>& instructions)
{
  for(size_t i = 0; i < instructionsJson->ArrayElements.Size(); ++i)
  {
    auto&& element = instructionsJson->ArrayElements[i];
    ParseInstruction(element, instructions);
  }
}

void SpirVInstructions::ParseInstruction(Lightning::JsonValue* instructionJson, HashMap<String, Instruction>& instructions)
{
  Lightning::JsonValue* opNameGrammar = instructionJson->GetMember("opname");
  Lightning::JsonValue* opCodeGrammar = instructionJson->GetMember("opcode");
  if(opNameGrammar == nullptr || opCodeGrammar == nullptr)
    return;

  String opName = opNameGrammar->AsString();
  Instruction& instruction = instructions[opName];
  instruction.mName = opName;
  instruction.mOpCode = opCodeGrammar->AsInteger();

  Lightning::JsonValue* operandsGrammar = instructionJson->GetMember("operands", Lightning::JsonErrorMode::DefaultValue);
  if(operandsGrammar != nullptr)
    ParseOperands(operandsGrammar, instruction.mOperands);
}

void SpirVInstructions::ParseOperands(Lightning::JsonValue* operandsJson, Array<Instruction::Operand>& operands)
{
  for(size_t i = 0; i < operandsJson->ArrayElements.Size(); ++i)
  {
    Lightning::JsonValue* operandGrammar = operandsJson->ArrayElements[i];
    Instruction::Operand& operand = operands.PushBack();

    Lightning::JsonValue* kind = operandGrammar->GetMember("kind", Lightning::JsonErrorMode::DefaultValue);
    if(kind != nullptr)
      operand.mKind = kind->AsString();
    Lightning::JsonValue* name = operandGrammar->GetMember("name", Lightning::JsonErrorMode::DefaultValue);
    if(name != nullptr)
      operand.mName = name->AsString();
  }
}

//-------------------------------------------------------------------SpirVDefinitions
void SpirVDefinitions::LoadSimplifiedDefaults()
{
  mInstructions.LoadSimplifiedDefaults();
  mExtensionInstructions["GLSL.std.450"].LoadSimplifiedGlsl450Defaults();
}

void SpirVDefinitions::LoadCore(StringParam filePath)
{
  Lightning::CompilationErrors errors;
  Lightning::JsonValue* valueRoot = Lightning::JsonReader::ReadIntoTreeFromFile(errors, filePath, nullptr);
  if(valueRoot == nullptr)
    return;

  mInstructions.Load(valueRoot);
}

void SpirVDefinitions::LoadExtension(StringParam extensionName, StringParam filePath)
{
  Lightning::CompilationErrors errors;
  Lightning::JsonValue* valueRoot = Lightning::JsonReader::ReadIntoTreeFromFile(errors, filePath, nullptr);
  if(valueRoot == nullptr)
    return;

  mExtensionInstructions[extensionName].Load(valueRoot);
}

}//namespace Plasma
// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class LightningSpirVFrontEnd;
class LightningSpirVFrontEndContext;
class EntryPointInfo;

void ValidateEntryPoint(LightningSpirVFrontEnd* translator,
                        Lightning::GenericFunctionNode* node,
                        LightningSpirVFrontEndContext* context);
void ValidateBasicEntryPoint(LightningSpirVFrontEnd* translator,
                             Lightning::GenericFunctionNode* node,
                             LightningSpirVFrontEndContext* context);
void ValidateGeometryEntryPoint(LightningSpirVFrontEnd* translator,
                                Lightning::GenericFunctionNode* node,
                                LightningSpirVFrontEndContext* context);

} // namespace Plasma

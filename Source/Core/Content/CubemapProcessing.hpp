// MIT Licensed (see LICENSE.md).

#pragma once

namespace Plasma
{

void ExtractCubemapFaces(Status& status,
                         Array<MipHeader>& mipHeaders,
                         Array<::byte*>& imageData,
                         TextureFormat::Enum format);
void MipmapCubemap(Array<MipHeader>& mipHeaders, Array<::byte*>& imageData, TextureFormat::Enum format, bool compressed);

} // namespace Plasma

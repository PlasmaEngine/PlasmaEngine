#pragma once

namespace Plasma
{
    inline VkFormat GetImageFormat(TextureFormat::Enum format)
    {
        switch (format)
        {
        case TextureFormat::R8: return VK_FORMAT_R8_UNORM;
        case TextureFormat::RG8: return VK_FORMAT_R8G8_UNORM;
        case TextureFormat::RGB8: return VK_FORMAT_R8G8B8_UNORM;
        case TextureFormat::RGBA8: return VK_FORMAT_R8G8B8A8_UNORM;

        case TextureFormat::R16: return VK_FORMAT_R16_UNORM;
        case TextureFormat::RG16: return VK_FORMAT_R16G16_UNORM;
        case TextureFormat::RGB16: return VK_FORMAT_R16G16B16_UNORM;
        case TextureFormat::RGBA16: return VK_FORMAT_R16G16B16A16_UNORM;

        case TextureFormat::R16f: return VK_FORMAT_R16_SFLOAT;
        case TextureFormat::RG16f: return VK_FORMAT_R16G16_SFLOAT;
        case TextureFormat::RGB16f: return VK_FORMAT_R16G16B16_SFLOAT;
        case TextureFormat::RGBA16f: return VK_FORMAT_R16G16B16A16_SFLOAT;

        case TextureFormat::R32f: return VK_FORMAT_R32_SFLOAT;
        case TextureFormat::RG32f: return VK_FORMAT_R32G32_SFLOAT;
        case TextureFormat::RGB32f: return VK_FORMAT_R32G32B32_SFLOAT;
        case TextureFormat::RGBA32f: return VK_FORMAT_R32G32B32A32_SFLOAT;

        case TextureFormat::SRGB8: return VK_FORMAT_R8G8B8_SRGB;
        case TextureFormat::SRGB8A8: return VK_FORMAT_R8G8B8A8_SRGB;

        case TextureFormat::Depth16: return VK_FORMAT_D16_UNORM;
        case TextureFormat::Depth32: return VK_FORMAT_D32_SFLOAT;
        case TextureFormat::Depth32f: return VK_FORMAT_D32_SFLOAT;
        case TextureFormat::Depth24Stencil8: return VK_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::Depth32fStencil8Pad24: return VK_FORMAT_D32_SFLOAT_S8_UINT;
        default:
            return VK_FORMAT_UNDEFINED;
        }
    }
}
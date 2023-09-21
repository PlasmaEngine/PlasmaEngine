// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

#include "TracyOpenGL.hpp"

#ifdef PlasmaTargetOsEmscripten
#  define PlasmaWebgl
#else
#  define PlasmaGl
#endif

#ifdef PlasmaGl
#  define PlasmaIfGl(X) X
#else
#  define PlasmaIfGl(X)
#endif

#ifdef PlasmaWebgl
#  define PlasmaIfWebgl(X) X
#else
#  define PlasmaIfWebgl(X)
#endif

static const size_t cMaxDrawBuffers = 4;

// As Of NVidia Driver 302 exporting this symbol will enable GPU hardware
// accelerated graphics when using Optimus (Laptop NVidia gpu / Intel HD auto
// switching). This is important for two reasons first is performance and second
// is stability since Intel seems to have a fair amount of bugs and crashes in
// their OpenGl drivers
extern "C" {
PlasmaExport int NvOptimusEnablement = 0x00000001;
}

// temporary to prevent string constructions every frame
// RenderQueue structures should have semantics for setting shader parameters
namespace
{
    const Plasma::String cFrameTime("FrameData.FrameTime");
    const Plasma::String cLogicTime("FrameData.LogicTime");
    const Plasma::String cNearPlane("CameraData.NearPlane");
    const Plasma::String cFarPlane("CameraData.FarPlane");
    const Plasma::String cViewportSize("CameraData.ViewportSize");
    const Plasma::String cInverseViewportSize("CameraData.InverseViewportSize");
    const Plasma::String cObjectWorldPosition("CameraData.ObjectWorldPosition");

    const Plasma::String cLocalToWorld("TransformData.LocalToWorld");
    const Plasma::String cWorldToLocal("TransformData.WorldToLocal");
    const Plasma::String cWorldToView("TransformData.WorldToView");
    const Plasma::String cViewToWorld("TransformData.ViewToWorld");
    const Plasma::String cLocalToView("TransformData.LocalToView");
    const Plasma::String cLastLocalToView("TransformData.LastLocalToView");
    const Plasma::String cViewToLocal("TransformData.ViewToLocal");
    const Plasma::String cLocalToWorldNormal("TransformData.LocalToWorldNormal");
    const Plasma::String cWorldToLocalNormal("TransformData.WorldToLocalNormal");
    const Plasma::String cLocalToViewNormal("TransformData.LocalToViewNormal");
    const Plasma::String cViewToLocalNormal("TransformData.ViewToLocalNormal");
    const Plasma::String cLocalToPerspective("TransformData.LocalToPerspective");
    const Plasma::String cViewToPerspective("TransformData.ViewToPerspective");
    const Plasma::String cPerspectiveToView("TransformData.PerspectiveToView");
    const Plasma::String cPlasmaPerspectiveToApiPerspective("TransformData.PlasmaPerspectiveToApiPerspective");

    const Plasma::String cSpriteSource("SpriteSource_SpriteSourceColor");
    const Plasma::String cSpriteSourceCubePreview("SpriteSource_TextureCubePreview");
} // namespace

namespace Plasma
{
    void GLAPIENTRY EmptyUniformFunc(GLint, GLsizei, const void*)
    {
    }

    const bool cTransposeMatrices = !(ColumnBasis == 1);

    struct GlTextureEnums
    {
        GLint mInternalFormat;
        GLint mFormat;
        GLint mType;
    };

    GlTextureEnums gTextureEnums[] = {
        {/* internalFormat    , format            , type */}, // None
        {GL_R8, GL_RED, GL_UNSIGNED_BYTE}, // R8
        {GL_RG8, GL_RG, GL_UNSIGNED_BYTE}, // RG8
        {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE}, // RGB8
        {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, // RGBA8
        {GL_R16, GL_RED, GL_UNSIGNED_SHORT}, // R16
        {GL_RG16, GL_RG, GL_UNSIGNED_SHORT}, // RG16
        {GL_RGB16, GL_RGB, GL_UNSIGNED_SHORT}, // RGB16
        {GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT}, // RGBA16
        {GL_R16F, GL_RED, GL_HALF_FLOAT}, // R16f
        {GL_RG16F, GL_RG, GL_HALF_FLOAT}, // RG16f
        {GL_RGB16F, GL_RGB, GL_HALF_FLOAT}, // RGB16f
        {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT}, // RGBA16f
        {GL_R32F, GL_RED, GL_FLOAT}, // R32f
        {GL_RG32F, GL_RG, GL_FLOAT}, // RG32f
        {GL_RGB32F, GL_RGB, GL_FLOAT}, // RGB32f
        {GL_RGBA32F, GL_RGBA, GL_FLOAT}, // RGBA32f
        {GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE}, // SRGB8
        {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE}, // SRGB8A8
        {GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT}, // Depth16
        {GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT}, // Depth24
        {GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT}, // Depth32
        {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT}, // Depth32f
        {GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8}, // Depth24Stencil8
        {GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV} // Depth32fStencil8Pad24
    };

    void WebglConvertTextureFormat(AddTextureInfo* info)
    {
        // 16 integer formats are unsupported, fallback to half floats to preserve
        // data size.
        if (IsShortColorFormat(info->mFormat))
        {
            switch (info->mFormat)
            {
            case TextureFormat::R16:
                info->mFormat = TextureFormat::R16f;
                break;
            case TextureFormat::RG16:
                info->mFormat = TextureFormat::RG16f;
                break;
            case TextureFormat::RGB16:
                info->mFormat = TextureFormat::RGB16f;
                break;
            case TextureFormat::RGBA16:
                info->mFormat = TextureFormat::RGBA16f;
                break;
            default:
                return;
            }

            uint componentCount = GetPixelSize(info->mFormat) / sizeof(u16);

            for (uint i = 0; i < info->mMipCount; ++i)
            {
                MipHeader* mipHeader = info->mMipHeaders + i;
                u16* imageData = (u16*)(info->mImageData + mipHeader->mDataOffset);
                uint pixelCount = mipHeader->mWidth * mipHeader->mHeight;

                for (uint p = 0; p < pixelCount * componentCount; ++p)
                {
                    float normalized = imageData[p] / 65535.0f;
                    imageData[p] = HalfFloatConverter::ToHalfFloat(normalized);
                }
            }
        }
    }

    void WebglConvertRenderTargetFormat(AddTextureInfo* info)
    {
        // For unsupported target formats, fallback to formats that do not drop any
        // data. Formats are either converting to floats, adding an alpha channel, or
        // both.
        switch (info->mFormat)
        {
        case TextureFormat::R16:
            info->mFormat = TextureFormat::R16f;
            break;
        case TextureFormat::RG16:
            info->mFormat = TextureFormat::RG16f;
            break;
        case TextureFormat::RGB16:
            info->mFormat = TextureFormat::RGBA16f;
            break;
        case TextureFormat::RGBA16:
            info->mFormat = TextureFormat::RGBA16f;
            break;
        case TextureFormat::RGB16f:
            info->mFormat = TextureFormat::RGBA16f;
            break;
        case TextureFormat::RGB32f:
            info->mFormat = TextureFormat::RGBA32f;
            break;
        case TextureFormat::SRGB8:
            info->mFormat = TextureFormat::SRGB8A8;
            break;
        case TextureFormat::Depth32:
            info->mFormat = TextureFormat::Depth32f;
            break;
        default:
            break;
        }
    }

    GLint GlInternalFormat(TextureCompression::Enum compression)
    {
        switch (compression)
        {
        case TextureCompression::BC1:
            return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case TextureCompression::BC2:
            return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case TextureCompression::BC3:
            return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case TextureCompression::BC4:
            return GL_COMPRESSED_RED_RGTC1;
        case TextureCompression::BC5:
            return GL_COMPRESSED_RG_RGTC2;
        case TextureCompression::BC6:
            return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB;
        case TextureCompression::BC7: 
            return GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
        default:
            return 0;
        }
    }

    GLuint ToOpenglType(VertexElementType::Enum type)
    {
        switch (type)
        {
        case VertexElementType::Byte:
        case VertexElementType::NormByte:
            return GL_UNSIGNED_BYTE;

        case VertexElementType::Short:
        case VertexElementType::NormShort:
            return GL_UNSIGNED_SHORT;

        case VertexElementType::Half:
            return GL_HALF_FLOAT;
        case VertexElementType::Real:
            return GL_FLOAT;

        default:
            return 0;
        }
    }

    GLuint GlPrimitiveType(PrimitiveType::Enum value)
    {
        switch (value)
        {
        case PrimitiveType::Triangles:
            return GL_TRIANGLES;
        case PrimitiveType::Lines:
            return GL_LINES;
        case PrimitiveType::Points:
            return GL_POINTS;
        default:
            return 0;
        }
    }

    GLuint GlTextureType(TextureType::Enum value)
    {
        switch (value)
        {
        case TextureType::Texture2D:
            return GL_TEXTURE_2D;
        case TextureType::TextureCube:
            return GL_TEXTURE_CUBE_MAP;
        case TextureType::Texture3D:
            return GL_TEXTURE_3D;
        default:
            return 0;
        }
    }

    GLuint GlTextureFace(TextureFace::Enum value)
    {
        switch (value)
        {
        case TextureFace::None:
            return GL_TEXTURE_2D;
        case TextureFace::PositiveX:
            return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
        case TextureFace::PositiveY:
            return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
        case TextureFace::PositiveZ:
            return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
        case TextureFace::NegativeX:
            return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        case TextureFace::NegativeY:
            return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
        case TextureFace::NegativeZ:
            return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
        default:
            return 0;
        }
    }

    GLuint GlTextureAddressing(TextureAddressing::Enum value)
    {
        switch (value)
        {
        case TextureAddressing::Clamp:
            return GL_CLAMP_TO_EDGE;
        case TextureAddressing::Repeat:
            return GL_REPEAT;
        case TextureAddressing::Mirror:
            return GL_MIRRORED_REPEAT;
        default:
            return 0;
        }
    }

    GLuint GlTextureFilteringMin(TextureFiltering::Enum value)
    {
        switch (value)
        {
        case TextureFiltering::Nearest:
            return GL_NEAREST_MIPMAP_NEAREST;
        case TextureFiltering::Bilinear:
            return GL_LINEAR_MIPMAP_NEAREST;
        case TextureFiltering::Trilinear:
            return GL_LINEAR_MIPMAP_LINEAR;
        default:
            return 0;
        }
    }

    GLuint GlTextureFilteringMag(TextureFiltering::Enum value)
    {
        switch (value)
        {
        case TextureFiltering::Nearest:
            return GL_NEAREST;
        case TextureFiltering::Bilinear:
            return GL_LINEAR;
        case TextureFiltering::Trilinear:
            return GL_LINEAR;
        default:
            return 0;
        }
    }

    GLfloat GlTextureAnisotropy(TextureAnisotropy::Enum value)
    {
        switch (value)
        {
        case TextureAnisotropy::x1:
            return 1.0f;
        case TextureAnisotropy::x2:
            return 2.0f;
        case TextureAnisotropy::x4:
            return 4.0f;
        case TextureAnisotropy::x8:
            return 8.0f;
        case TextureAnisotropy::x16:
            return 16.0f;
        default:
            return 1.0f;
        }
    }

    GLuint GlTextureMipMapping(TextureMipMapping::Enum value)
    {
        switch (value)
        {
        case TextureMipMapping::None:
            return 0;
        case TextureMipMapping::PreGenerated:
            return 1000;
        case TextureMipMapping::GpuGenerated:
            return 1000;
        default:
            return 0;
        }
    }

    void CheckShader(GLuint shader, StringParam shaderCode)
    {
#ifdef PlasmaDebug
  GLint status = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE)
  {
    GLint infoLogLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar* strInfoLog = (GLchar*)alloca(infoLogLength + 1);
    glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
    PlasmaPrint("Compile Error\n%s\n", strInfoLog);

    static size_t sMaxPrints = 4;
    if (sMaxPrints > 0)
    {
      PlasmaPrint("\n************************************************************\n%"
             "s\n************************************************************\n",
             shaderCode.c_str());
      --sMaxPrints;
    }
  }
#endif
    }

    void SetClearData(void* clearData, TextureFormat::Enum format, Vec4 color, float depth)
    {
        switch (format)
        {
        case TextureFormat::RGB8:
        case TextureFormat::RGBA8:
            *static_cast<ByteColor*>(clearData) = ToByteColor(color);
            break;

        case TextureFormat::RGB32f:
        case TextureFormat::RGBA32f:
            *static_cast<Vec4*>(clearData) = color;
            break;

        case TextureFormat::Depth16:
            *static_cast<u16*>(clearData) = static_cast<u16>(static_cast<u16>(-1) * static_cast<double>(depth));
            break;

        case TextureFormat::Depth24:
        case TextureFormat::Depth32:
            *static_cast<uint*>(clearData) = static_cast<uint>(static_cast<uint>(-1) * static_cast<double>(depth));
            break;

        case TextureFormat::Depth32f:
            *static_cast<float*>(clearData) = depth;
            break;

        case TextureFormat::Depth24Stencil8:
            // not handled, but this function is not currently being used
            break;
        default:
            break;
        }
    }

    GLuint GlCullFace(CullMode::Enum value)
    {
        switch (value)
        {
        case CullMode::BackFace:
            return GL_BACK;
        case CullMode::FrontFace:
            return GL_FRONT;
        default:
            return 0;
        }
    }

    GLuint GlBlendFactor(BlendFactor::Enum value)
    {
        switch (value)
        {
        case BlendFactor::Plasma:
            return GL_ZERO;
        case BlendFactor::One:
            return GL_ONE;
        case BlendFactor::SourceColor:
            return GL_SRC_COLOR;
        case BlendFactor::InvSourceColor:
            return GL_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DestColor:
            return GL_DST_COLOR;
        case BlendFactor::InvDestColor:
            return GL_ONE_MINUS_DST_COLOR;
        case BlendFactor::SourceAlpha:
            return GL_SRC_ALPHA;
        case BlendFactor::InvSourceAlpha:
            return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DestAlpha:
            return GL_DST_ALPHA;
        case BlendFactor::InvDestAlpha:
            return GL_ONE_MINUS_DST_ALPHA;
        case BlendFactor::SourceAlphaSaturate:
            return GL_SRC_ALPHA_SATURATE;
        default:
            return 0;
        }
    }

    GLuint GlBlendEquation(BlendEquation::Enum blendEquation)
    {
        switch (blendEquation)
        {
        case BlendEquation::Add:
            return GL_FUNC_ADD;
        case BlendEquation::Subtract:
            return GL_FUNC_SUBTRACT;
        case BlendEquation::ReverseSubtract:
            return GL_FUNC_REVERSE_SUBTRACT;
        case BlendEquation::Min:
            return GL_MIN;
        case BlendEquation::Max:
            return GL_MAX;
        default:
            return 0;
        }
    }

    GLboolean GlDepthMode(DepthMode::Enum value)
    {
        switch (value)
        {
        case DepthMode::Read:
            return false;
        case DepthMode::Write:
            return true;
        default:
            return false;
        }
    }

    GLuint GlStencilOp(StencilOp::Enum value)
    {
        switch (value)
        {
        case StencilOp::Plasma:
            return GL_ZERO;
        case StencilOp::Keep:
            return GL_KEEP;
        case StencilOp::Replace:
            return GL_REPLACE;
        case StencilOp::Invert:
            return GL_INVERT;
        case StencilOp::Increment:
            return GL_INCR;
        case StencilOp::IncrementWrap:
            return GL_INCR_WRAP;
        case StencilOp::Decrement:
            return GL_DECR;
        case StencilOp::DecrementWrap:
            return GL_DECR_WRAP;
        default:
            return 0;
        }
    }

    GLuint GlCompareMode(TextureCompareMode::Enum compareMode)
    {
        switch (compareMode)
        {
        case TextureCompareMode::Disabled:
            return GL_NONE;
        case TextureCompareMode::Enabled:
            return GL_COMPARE_R_TO_TEXTURE;
        default:
            return 0;
        }
    }

    GLuint GlCompareFunc(TextureCompareFunc::Enum value)
    {
        switch (value)
        {
        case TextureCompareFunc::Never:
            return GL_NEVER;
        case TextureCompareFunc::Always:
            return GL_ALWAYS;
        case TextureCompareFunc::Less:
            return GL_LESS;
        case TextureCompareFunc::LessEqual:
            return GL_LEQUAL;
        case TextureCompareFunc::Greater:
            return GL_GREATER;
        case TextureCompareFunc::GreaterEqual:
            return GL_GEQUAL;
        case TextureCompareFunc::Equal:
            return GL_EQUAL;
        case TextureCompareFunc::NotEqual:
            return GL_NOTEQUAL;
        default:
            return 0;
        }
    }

    void SetBlendSettings(const BlendSettings& blendSettings)
    {
        switch (blendSettings.mBlendMode)
        {
        case BlendMode::Disabled:
            glDisable(GL_BLEND);
            break;
        case BlendMode::Enabled:
            glEnable(GL_BLEND);
            glBlendEquation(GlBlendEquation(blendSettings.mBlendEquation));
            glBlendFunc(GlBlendFactor(blendSettings.mSourceFactor), GlBlendFactor(blendSettings.mDestFactor));
            break;
        case BlendMode::Separate:
            glEnable(GL_BLEND);
            glBlendEquationSeparate(GlBlendEquation(blendSettings.mBlendEquation),
                                    GlBlendEquation(blendSettings.mBlendEquationAlpha));
            glBlendFuncSeparate(GlBlendFactor(blendSettings.mSourceFactor),
                                GlBlendFactor(blendSettings.mDestFactor),
                                GlBlendFactor(blendSettings.mSourceFactorAlpha),
                                GlBlendFactor(blendSettings.mDestFactorAlpha));
            break;
        }
    }

    void SetRenderSettings(const RenderSettings& renderSettings, bool drawBuffersBlend)
    {
        switch (renderSettings.mCullMode)
        {
        case CullMode::Disabled:
            glDisable(GL_CULL_FACE);
            break;
        case CullMode::BackFace:
        case CullMode::FrontFace:
            glEnable(GL_CULL_FACE);
            glCullFace(GlCullFace(renderSettings.mCullMode));
            break;
        }

        if (renderSettings.mSingleColorTarget || drawBuffersBlend == false)
        {
            SetBlendSettings(renderSettings.mBlendSettings[0]);
        }
        else
        {
            for (uint i = 0; i < cMaxDrawBuffers; ++i)
            {
                const BlendSettings& blendSettings = renderSettings.mBlendSettings[i];
                switch (blendSettings.mBlendMode)
                {
                case BlendMode::Disabled:
                    glDisablei(GL_BLEND, i);
                    break;
                case BlendMode::Enabled:
                    glEnablei(GL_BLEND, i);
                    glBlendEquationi(i, GlBlendEquation(blendSettings.mBlendEquation));
                    glBlendFunci(i, GlBlendFactor(blendSettings.mSourceFactor),
                                 GlBlendFactor(blendSettings.mDestFactor));
                    break;
                case BlendMode::Separate:
                    glEnablei(GL_BLEND, i);
                    glBlendEquationSeparatei(
                        i, GlBlendEquation(blendSettings.mBlendEquation),
                        GlBlendEquation(blendSettings.mBlendEquationAlpha));
                    glBlendFuncSeparatei(i,
                                         GlBlendFactor(blendSettings.mSourceFactor),
                                         GlBlendFactor(blendSettings.mDestFactor),
                                         GlBlendFactor(blendSettings.mSourceFactorAlpha),
                                         GlBlendFactor(blendSettings.mDestFactorAlpha));
                    break;
                }
            }
        }

        const DepthSettings& depthSettings = renderSettings.mDepthSettings;
        switch (depthSettings.mDepthMode)
        {
        case DepthMode::Disabled:
            glDisable(GL_DEPTH_TEST);
            break;
        case DepthMode::Read:
        case DepthMode::Write:
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GlDepthMode(depthSettings.mDepthMode));
            glDepthFunc(GlCompareFunc(depthSettings.mDepthCompareFunc));
            break;
        }

        switch (depthSettings.mStencilMode)
        {
        case StencilMode::Disabled:
            glDisable(GL_STENCIL_TEST);
            glStencilMask(0);
            break;
        case StencilMode::Enabled:
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GlCompareFunc(depthSettings.mStencilCompareFunc),
                          depthSettings.mStencilTestValue,
                          depthSettings.mStencilReadMask);
            glStencilOp(GlStencilOp(depthSettings.mStencilFailOp),
                        GlStencilOp(depthSettings.mDepthFailOp),
                        GlStencilOp(depthSettings.mDepthPassOp));
            glStencilMask(depthSettings.mStencilWriteMask);
            break;
        case StencilMode::Separate:
            glEnable(GL_STENCIL_TEST);
            glStencilFuncSeparate(GL_FRONT,
                                  GlCompareFunc(depthSettings.mStencilCompareFunc),
                                  depthSettings.mStencilTestValue,
                                  depthSettings.mStencilReadMask);
            glStencilOpSeparate(GL_FRONT,
                                GlStencilOp(depthSettings.mStencilFailOp),
                                GlStencilOp(depthSettings.mDepthFailOp),
                                GlStencilOp(depthSettings.mDepthPassOp));
            glStencilMaskSeparate(GL_FRONT, depthSettings.mStencilWriteMask);
            glStencilFuncSeparate(GL_BACK,
                                  GlCompareFunc(depthSettings.mStencilCompareFuncBackFace),
                                  depthSettings.mStencilTestValueBackFace,
                                  depthSettings.mStencilReadMaskBackFace);
            glStencilOpSeparate(GL_BACK,
                                GlStencilOp(depthSettings.mStencilFailOpBackFace),
                                GlStencilOp(depthSettings.mDepthFailOpBackFace),
                                GlStencilOp(depthSettings.mDepthPassOpBackFace));
            glStencilMaskSeparate(GL_BACK, depthSettings.mStencilWriteMaskBackFace);
            break;
        }

        switch (renderSettings.mScissorMode)
        {
        case ScissorMode::Disabled:
            glDisable(GL_SCISSOR_TEST);
            break;
        case ScissorMode::Enabled:
            glEnable(GL_SCISSOR_TEST);
            break;
        }
    }

    void BindTexture(TextureType::Enum textureType, uint textureSlot, uint textureId, bool samplerObjects)
    {
        // Clear anything bound to this texture unit
        glActiveTexture(GL_TEXTURE0 + textureSlot);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glBindTexture(GL_TEXTURE_3D, 0);
        if (samplerObjects)
            glBindSampler(textureSlot, 0);
        // Bind texture
        glBindTexture(GlTextureType(textureType), textureId);
    }

    void CheckFramebufferStatus()
    {
#ifdef PlasmaDebug
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  switch (status)
  {
  case GL_FRAMEBUFFER_UNSUPPORTED:
    DebugPrint("Unsupported framebuffer format\n");
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    DebugPrint("Framebuffer incomplete, missing attachment\n");
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    DebugPrint("Framebuffer incomplete, incomplete attachment\n");
    break;
  }
  ErrorIf(status != GL_FRAMEBUFFER_COMPLETE, "Framebuffer incomplete");
#endif
    }

    void SetSingleRenderTargets(GLuint fboId, TextureRenderData** colorTargets, TextureRenderData* depthTarget)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fboId);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glDrawBuffer(GL_NONE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

        GlTextureRenderData* colorRenderData = static_cast<GlTextureRenderData*>(colorTargets[0]);
        if (colorRenderData != nullptr)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorRenderData->mId, 0);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
        }

        GlTextureRenderData* depthRenderData = static_cast<GlTextureRenderData*>(depthTarget);
        if (depthRenderData != nullptr)
        {
            if (IsDepthStencilFormat(depthRenderData->mFormat))
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthRenderData->mId,
                                       0);
            else
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthRenderData->mId, 0);
        }

        CheckFramebufferStatus();
    }

    void SetMultiRenderTargets(GLuint fboId, TextureRenderData** colorTargets, TextureRenderData* depthTarget)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fboId);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

        GLenum drawBuffers[cMaxDrawBuffers];

        for (uint i = 0; i < cMaxDrawBuffers; ++i)
        {
            GlTextureRenderData* colorRenderData = static_cast<GlTextureRenderData*>(colorTargets[i]);
            if (colorRenderData != nullptr)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorRenderData->mId,
                                       0);
                drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
            }
            else
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
                drawBuffers[i] = GL_NONE;
            }
        }

        // Set active buffers, some drivers do not work correctly if all are always
        // active
        glDrawBuffers(cMaxDrawBuffers, drawBuffers);

        GlTextureRenderData* depthRenderData = static_cast<GlTextureRenderData*>(depthTarget);
        if (depthRenderData != nullptr)
        {
            if (IsDepthStencilFormat(depthRenderData->mFormat))
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthRenderData->mId,
                                       0);
            else
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthRenderData->mId, 0);
        }

        CheckFramebufferStatus();
    }

    void StreamedVertexBuffer::Initialize()
    {
        mBufferSize = 1 << 18; // 256Kb, 1213 sprites at 216 bytes per sprite
        mCurrentBufferOffset = 0;

        glGenVertexArrays(1, &mVertexArray);
        glBindVertexArray(mVertexArray);

        glGenBuffers(1, &mVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, mBufferSize, nullptr, GL_STREAM_DRAW);

        glEnableVertexAttribArray(VertexSemantic::Position);
        glVertexAttribPointer(VertexSemantic::Position,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(StreamedVertex),
                              (void*)PlasmaOffsetOf(StreamedVertex, mPosition));
        glEnableVertexAttribArray(VertexSemantic::Uv);
        glVertexAttribPointer(
            VertexSemantic::Uv, 2, GL_FLOAT, GL_FALSE, sizeof(StreamedVertex),
            (void*)PlasmaOffsetOf(StreamedVertex, mUv));
        glEnableVertexAttribArray(VertexSemantic::Color);
        glVertexAttribPointer(VertexSemantic::Color,
                              4,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(StreamedVertex),
                              (void*)PlasmaOffsetOf(StreamedVertex, mColor));
        glEnableVertexAttribArray(VertexSemantic::UvAux);
        glVertexAttribPointer(VertexSemantic::UvAux,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(StreamedVertex),
                              (void*)PlasmaOffsetOf(StreamedVertex, mUvAux));

        glBindVertexArray(0);

        mPrimitiveType = PrimitiveType::Triangles;
        mActive = false;
    }

    void StreamedVertexBuffer::Destroy()
    {
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteVertexArrays(1, &mVertexArray);
    }

    void StreamedVertexBuffer::AddVertices(StreamedVertex* vertices, uint count, PrimitiveType::Enum primitiveType)
    {
        if (!mActive)
        {
            glBindVertexArray(mVertexArray);
            glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
            mActive = true;
        }

        if (primitiveType != mPrimitiveType)
        {
            FlushBuffer(false);
            mPrimitiveType = primitiveType;
        }

        uint uploadSize = sizeof(StreamedVertex) * count;
        if (mCurrentBufferOffset + uploadSize > mBufferSize)
        {
            FlushBuffer(false);
            // If upload size is larger than the entire buffer then break it into
            // multiple draws
            while (uploadSize > mBufferSize)
            {
                uint verticesPerPrimitive = primitiveType + 1;
                uint primitiveSize = sizeof(StreamedVertex) * verticesPerPrimitive;
                uint maxPrimitiveCount = mBufferSize / primitiveSize;
                uint maxByteCount = maxPrimitiveCount * primitiveSize;

                mCurrentBufferOffset = maxByteCount;
                glBufferSubData(GL_ARRAY_BUFFER, 0, mCurrentBufferOffset, vertices);
                FlushBuffer(false);
                // Move pointer forward by byte count, below condition will grab this new
                // value
                vertices = (StreamedVertex*)((char*)vertices + maxByteCount);
                uploadSize -= maxByteCount;
            }
        }

        glBufferSubData(GL_ARRAY_BUFFER, mCurrentBufferOffset, uploadSize, vertices);
        mCurrentBufferOffset += uploadSize;
    }

    void StreamedVertexBuffer::AddVertices(StreamedVertexArray& vertices,
                                           uint start,
                                           uint count,
                                           PrimitiveType::Enum primitiveType)
    {
        while (count > 0)
        {
            uint verticesPerPrimitive = primitiveType + 1;

            // Get the maximum number of contiguous whole primitives.
            uint indexInBucket = start & StreamedVertexArray::BucketMask;
            uint contiguousCount = StreamedVertexArray::BucketSize - indexInBucket;
            uint uploadCount = Math::Min(contiguousCount, count);

            uint remainder = uploadCount % verticesPerPrimitive;
            uploadCount -= remainder;

            AddVertices(&vertices[start], uploadCount, primitiveType);
            start += uploadCount;
            count -= uploadCount;

            if (remainder != 0)
            {
                ErrorIf(count < verticesPerPrimitive, "Bad count if it does not have a whole number of primitives.");
                // Manually populate one primitive over the block array boundary.
                StreamedVertex primitive[3];
                for (uint i = 0; i < verticesPerPrimitive; ++i)
                    primitive[i] = vertices[start + i];

                AddVertices(primitive, verticesPerPrimitive, primitiveType);
                start += verticesPerPrimitive;
                count -= verticesPerPrimitive;
            }
        }
    }

    void StreamedVertexBuffer::FlushBuffer(bool deactivate)
    {
        TracyGpuZone("FlushBuffers");
        if (mCurrentBufferOffset > 0)
        {
            if (mPrimitiveType == PrimitiveType::Triangles)
                glDrawArrays(GL_TRIANGLES, 0, mCurrentBufferOffset / sizeof(StreamedVertex));
            else if (mPrimitiveType == PrimitiveType::Lines)
                glDrawArrays(GL_LINES, 0, mCurrentBufferOffset / sizeof(StreamedVertex));
            else if (mPrimitiveType == PrimitiveType::Points)
                glDrawArrays(GL_POINTS, 0, mCurrentBufferOffset / sizeof(StreamedVertex));
            mCurrentBufferOffset = 0;
        }

        if (deactivate && mActive)
        {
            glBindVertexArray(0);
            glLineWidth(1.0f);
            mActive = false;
        }
    }

    void OpenglRenderer::Initialize(OsHandle windowHandle, OsHandle deviceContext, OsHandle renderContext,
                                    String& error)
    {
        mVsync = false;

        mWindow = windowHandle;
        mDeviceContext = deviceContext;
        mRenderContext = renderContext;

        // Read the OpenGL version support
        const char* gl_version = (const char*)glGetString(GL_VERSION);
        if (gl_version == nullptr)
        {
            error = "Unable to query OpenGL version. "
                "Please update your computer's graphics drivers or verify that "
                "your graphics card supports OpenGL.";
            return;
        }

        const char* gl_sl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
        const char* gl_vendor = (const char*)glGetString(GL_VENDOR);
        const char* gl_renderer = (const char*)glGetString(GL_RENDERER);
        const char* gl_extensions = (const char*)glGetString(GL_EXTENSIONS);

        PlasmaPrint("OpenGL Version          : %s\n", gl_version ? gl_version : "(no data)");
        PlasmaPrint("OpenGL Shading Language : %s\n", gl_sl_version ? gl_sl_version : "(no data)");
        PlasmaPrint("OpenGL Vendor           : %s\n", gl_vendor ? gl_vendor : "(no data)");
        PlasmaPrint("OpenGL Renderer         : %s\n", gl_renderer ? gl_renderer : "(no data)");

        // Initialize glew
        GLenum glewInitStatus = glewInit();
        if (glewInitStatus != GLEW_OK)
        {
            error = String::Format("GLEW failed to initialize with error: %d", glewInitStatus);
            return;
        }

#if defined (PLASMA_PLATFORM_WINDOWS)
        int attribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_FLAGS_ARB, 0,
            0
        };
    	
        HGLRC newRenderContext = wglCreateContextAttribsARB((HDC)deviceContext, 0, attribs);
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext((HGLRC)renderContext);
        wglMakeCurrent((HDC)deviceContext, newRenderContext);
#endif
    	
        TracyGpuContext;

#ifdef PlasmaWebgl
  // glewIsSupported on emscripten doesn't emulate desktop gl extension queries.
  bool version_4_0 = true;
  bool texture_compression = false;
  bool draw_buffers_blend = false;
  bool sampler_objects = true;
#else
        bool version_4_0 = glewIsSupported("GL_VERSION_4_0");
        bool texture_compression = glewIsSupported("GL_ARB_texture_compression");
        bool draw_buffers_blend = glewIsSupported("GL_ARB_draw_buffers_blend");
        bool sampler_objects = glewIsSupported("GL_ARB_sampler_objects");
#endif

        PlasmaPrint("OpenGL *Required Extensions\n");
        PlasmaPrint("OpenGL *(GL_VERSION_4_0) Shader Program support                 : %s\n",
                    version_4_0 ? "True" : "False");

        PlasmaPrint("OpenGL (GL_ARB_texture_compression) Texture Compression support : %s\n",
                    texture_compression ? "True" : "False");
        PlasmaPrint("OpenGL (GL_ARB_draw_buffers_blend) Multi Target Blend support   : %s\n",
                    draw_buffers_blend ? "True" : "False");
        PlasmaPrint("OpenGL (GL_ARB_sampler_objects) Sampler Object support          : %s\n",
                    sampler_objects ? "True" : "False");

        PlasmaPrint("OpenGL All Extensions : %s\n", gl_extensions ? gl_extensions : "(no data)");

        mDriverSupport.mTextureCompression = texture_compression;
        mDriverSupport.mMultiTargetBlend = draw_buffers_blend;
        mDriverSupport.mSamplerObjects = sampler_objects;

        // Intel integrated graphics does not render correctly with borderless
        // Window's aero on OpenGL.
        String vendorString = gl_vendor;
        if (vendorString.Contains("Intel"))
            mDriverSupport.mIntel = true;

        // V-Sync off by default
        plGlSetSwapInterval(this, 0);

        // No padding
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#if !defined(PlasmaWebgl)
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_TEXTURE_CUBE_MAP);
        glEnable((GL_TEXTURE_3D));
#endif

        if (glewIsSupported("GL_ARB_seamless_cube_map"))
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        // GLint maxAttributes;
        // glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttributes); // 16
        // for (int i = 0; i < maxAttributes; ++i)
        //   glEnableVertexAttribArray(i);

        // GLint maxAttach;
        // glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);

        // GLint maxTexUnits;
        // glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexUnits);

        // GLfloat maxAnisotropy;
        // glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);

        mLazyShaderCompilation = true;

        mActiveShader = 0;
        mActiveMaterial = 0;
        mActiveTexture = 0;

        mCurrentLineWidth = 1.0f;
        mClipMode = false;
        mCurrentClip = Vec4(0, 0, 0, 0);

        // buffer for fullscreen triangle
        StreamedVertex triangleVertices[] = {
            {Vec3(-1, 3, 0), Vec2(0, -1), Vec4()},
            {Vec3(-1, -1, 0), Vec2(0, 1), Vec4()},
            {Vec3(3, -1, 0), Vec2(2, 1), Vec4()},
        };

        uint triangleIndices[] = {0, 1, 2};

        glGenVertexArrays(1, &mTriangleArray);
        glBindVertexArray(mTriangleArray);

        glGenBuffers(1, &mTriangleVertex);
        glBindBuffer(GL_ARRAY_BUFFER, mTriangleVertex);
        glBufferData(GL_ARRAY_BUFFER, sizeof(StreamedVertex) * 3, triangleVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(VertexSemantic::Position);
        glVertexAttribPointer(VertexSemantic::Position,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(StreamedVertex),
                              (void*)PlasmaOffsetOf(StreamedVertex, mPosition));
        glEnableVertexAttribArray(VertexSemantic::Uv);
        glVertexAttribPointer(
            VertexSemantic::Uv, 2, GL_FLOAT, GL_FALSE, sizeof(StreamedVertex),
            (void*)PlasmaOffsetOf(StreamedVertex, mUv));

        glGenBuffers(1, &mTriangleIndex);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTriangleIndex);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * 3, triangleIndices, GL_STATIC_DRAW);

        glBindVertexArray(0);

        // frame buffers
        glGenFramebuffers(1, &mSingleTargetFbo);
        glGenFramebuffers(1, &mMultiTargetFbo);

        // Function invocations will fail if calling convention is not correct,
        // currently using GLAPIENTRY
        mUniformFunctions[ShaderInputType::Invalid] = EmptyUniformFunc;
        mUniformFunctions[ShaderInputType::Bool] = (UniformFunction)glUniform1iv;
        mUniformFunctions[ShaderInputType::Int] = (UniformFunction)glUniform1iv;
        mUniformFunctions[ShaderInputType::IntVec2] = (UniformFunction)glUniform2iv;
        mUniformFunctions[ShaderInputType::IntVec3] = (UniformFunction)glUniform3iv;
        mUniformFunctions[ShaderInputType::IntVec4] = (UniformFunction)glUniform4iv;
        mUniformFunctions[ShaderInputType::Float] = (UniformFunction)glUniform1fv;
        mUniformFunctions[ShaderInputType::Vec2] = (UniformFunction)glUniform2fv;
        mUniformFunctions[ShaderInputType::Vec3] = (UniformFunction)glUniform3fv;
        mUniformFunctions[ShaderInputType::Vec4] = (UniformFunction)glUniform4fv;
        mUniformFunctions[ShaderInputType::Mat3] = EmptyUniformFunc;
        mUniformFunctions[ShaderInputType::Mat4] = EmptyUniformFunc;
        mUniformFunctions[ShaderInputType::Texture] = (UniformFunction)glUniform1iv;

        mStreamedVertexBuffer.Initialize();

#define PlasmaGlVertexIn PlasmaIfGl("in") PlasmaIfWebgl("attribute")
#define PlasmaGlVertexOut PlasmaIfGl("out") PlasmaIfWebgl("varying")
#define PlasmaGlPixelIn PlasmaIfGl("in") PlasmaIfWebgl("varying")

        // This will most likley have to change to use uniform buffers
        String loadingShaderVertex = PlasmaIfGl("#version 150\n") PlasmaIfWebgl("#version 100\n")
            PlasmaIfWebgl("precision mediump float;\n") "uniform mat4 Transform;\n"
            "uniform mat3 "
            "UvTransform;\n" PlasmaGlVertexIn " vec3 LocalPosition;\n" PlasmaGlVertexIn
            " vec2 Uv;\n" PlasmaGlVertexOut " vec2 psInUv;\n"
            "void main(void)\n"
            "{\n"
            "  psInUv = (vec3(Uv, 1.0) * "
            "UvTransform).xy;\n"
            "  gl_Position = vec4(LocalPosition, "
            "1.0) * Transform;\n"
            "}";

        String loadingShaderPixel = PlasmaIfGl("#version 150\n") PlasmaIfWebgl("#version 100\n")
            PlasmaIfWebgl("precision mediump float;\n") "uniform sampler2D Texture;\n"
            "uniform float Alpha;\n" PlasmaGlPixelIn " vec2 psInUv;\n"
            "void main(void)\n"
            "{\n"
            "  vec2 uv = vec2(psInUv.x, 1.0 - "
            "psInUv.y);\n"
            "  gl_FragColor = texture2D(Texture, "
            "uv);\n"
            "  gl_FragColor.xyz *= Alpha;\n"
            "}";

        CreateShader(loadingShaderVertex, String(), loadingShaderPixel, mLoadingShader);
    }

    void OpenglRenderer::Shutdown()
    {
        ErrorIf(mGlShaders.Empty() == false, "Not all shaders were deleted.");
        ErrorIf(mShaderEntries.Empty() == false, "Not all shaders were deleted.");

        DelayedRenderDataDestruction();

        glDeleteFramebuffers(1, &mSingleTargetFbo);
        glDeleteFramebuffers(1, &mMultiTargetFbo);

        glDeleteVertexArrays(1, &mTriangleArray);
        glDeleteBuffers(1, &mTriangleVertex);
        glDeleteBuffers(1, &mTriangleIndex);

        glDeleteProgram(mLoadingShader);

        mStreamedVertexBuffer.Destroy();

        forRange(GLuint sampler, mSamplers.Values())
            glDeleteSamplers(1, &sampler);
        mSamplers.Clear();
    }

    void OpenglRenderer::BuildOrthographicTransform(
        Mat4Ref matrix, float size, float aspect, float nearPlane, float farPlane)
    {
        BuildOrthographicTransformGl(matrix, size, aspect, nearPlane, farPlane);
    }

    void OpenglRenderer::BuildPerspectiveTransform(Mat4Ref matrix, float fov, float aspect, float nearPlane,
                                                   float farPlane)
    {
        BuildPerspectiveTransformGl(matrix, fov, aspect, nearPlane, farPlane);
    }

    bool OpenglRenderer::YInvertImageData(TextureType::Enum type)
    {
        // OpenGL convention for cubemaps is not Y-inverted
        return (type != TextureType::TextureCube);
    }

    GlMaterialRenderData* OpenglRenderer::CreateMaterialRenderData()
    {
        GlMaterialRenderData* renderData = new GlMaterialRenderData();
        renderData->mResourceId = 0;
        return renderData;
    }

    GlMeshRenderData* OpenglRenderer::CreateMeshRenderData()
    {
        GlMeshRenderData* renderData = new GlMeshRenderData();
        renderData->mVertexBuffer = 0;
        renderData->mIndexBuffer = 0;
        renderData->mVertexArray = 0;
        renderData->mIndexCount = 0;
        return renderData;
    }

    GlTextureRenderData* OpenglRenderer::CreateTextureRenderData()
    {
        GlTextureRenderData* renderData = new GlTextureRenderData();
        renderData->mId = 0;
        renderData->mFormat = TextureFormat::None;
        return renderData;
    }

    void OpenglRenderer::AddMaterial(AddMaterialInfo* info)
    {
        GlMaterialRenderData* renderData = static_cast<GlMaterialRenderData*>(info->mRenderData);

        renderData->mCompositeName = info->mCompositeName;
        renderData->mResourceId = info->mMaterialId;
    }

    void OpenglRenderer::AddMesh(AddMeshInfo* info)
    {
        GlMeshRenderData* renderData = static_cast<GlMeshRenderData*>(info->mRenderData);
        if (renderData->mVertexArray != 0)
        {
            glDeleteVertexArrays(1, &renderData->mVertexArray);
            glDeleteBuffers(1, &renderData->mVertexBuffer);
            glDeleteBuffers(1, &renderData->mIndexBuffer);
        }

        if (info->mVertexData == nullptr)
        {
            renderData->mVertexBuffer = 0;
            renderData->mIndexBuffer = 0;
            renderData->mVertexArray = 0;
            renderData->mIndexCount = info->mIndexCount;
            renderData->mPrimitiveType = info->mPrimitiveType;
            return;
        }

        GLuint vertexArray;
        glGenVertexArrays(1, &vertexArray);
        glBindVertexArray(vertexArray);

        GLuint vertexBuffer;
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, info->mVertexCount * info->mVertexSize, info->mVertexData, GL_STATIC_DRAW);

        forRange(VertexAttribute& element, info->mVertexAttributes.All())
        {
            bool normalized = element.mType >= VertexElementType::NormByte;
            glEnableVertexAttribArray(element.mSemantic);
            if (element.mType == VertexElementType::Byte || element.mType == VertexElementType::Short)
                glVertexAttribIPointer(element.mSemantic,
                                       element.mCount,
                                       ToOpenglType(element.mType),
                                       info->mVertexSize,
                                       (void*)static_cast<uintptr_t>(element.mOffset));
            else
                glVertexAttribPointer(element.mSemantic,
                                      element.mCount,
                                      ToOpenglType(element.mType),
                                      normalized,
                                      info->mVertexSize,
                                      (void*)static_cast<uintptr_t>(element.mOffset));
        }

        GLuint indexBuffer = 0;
        if (info->mIndexData != nullptr)
        {
            glGenBuffers(1, &indexBuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, info->mIndexCount * info->mIndexSize, info->mIndexData,
                         GL_STATIC_DRAW);
        }

        glBindVertexArray(0);

        renderData->mVertexBuffer = vertexBuffer;
        renderData->mIndexBuffer = indexBuffer;
        renderData->mVertexArray = vertexArray;
        renderData->mIndexCount = info->mIndexCount;
        renderData->mPrimitiveType = info->mPrimitiveType;

        delete[] info->mVertexData;
        delete[] info->mIndexData;

        renderData->mBones.Assign(info->mBones.All());
    }

    void OpenglRenderer::AddTexture(AddTextureInfo* info)
    {
        GlTextureRenderData* renderData = static_cast<GlTextureRenderData*>(info->mRenderData);

        if (info->mFormat == TextureFormat::None)
        {
            if (renderData->mId != 0)
            {
                glDeleteTextures(1, &renderData->mId);
                renderData->mId = 0;
            }
        }
        else
        {
            if (info->mType != renderData->mType && renderData->mId != 0)
            {
                glDeleteTextures(1, &renderData->mId);
                renderData->mId = 0;
            }

            if (renderData->mId == 0)
                glGenTextures(1, &renderData->mId);

            BindTexture(info->mType, 0, renderData->mId, mDriverSupport.mSamplerObjects);

            // RenderTarget upload if data size is 0 (not calculated).
            // A texture resource with uploaded data will never set data size to 0.
            if (info->mTotalDataSize == 0)
            {
                PlasmaIfWebgl(WebglConvertRenderTargetFormat(info));
                GlTextureEnums glEnums = gTextureEnums[info->mFormat];

                switch(info->mType)
                {
                case TextureType::Texture3D:
                    glTexImage3D(GL_TEXTURE_3D,
                                0,
                                glEnums.mInternalFormat,
                                info->mWidth,
                                info->mHeight,
                                info->mDepth,
                                0,
                                glEnums.mFormat,
                                glEnums.mType,
                                nullptr);
                break;
                case TextureType::TextureCube:
                case TextureType::Texture2D: 
                    // Intentional fall through.
                default:
                {
                  // Rendering to cubemap is not implemented.
                  glTexImage2D(GL_TEXTURE_2D,
                               0,
                               glEnums.mInternalFormat,
                               info->mWidth,
                               info->mHeight,
                               0,
                               glEnums.mFormat,
                               glEnums.mType,
                               nullptr);
                  break;
                }
                }

               
            }
                // Do not try to reallocate texture data if no new data is given.
            else if (info->mImageData != nullptr)
            {
                PlasmaIfWebgl(WebglConvertTextureFormat(info));
                GlTextureEnums glEnums = gTextureEnums[info->mFormat];

                for (uint i = 0; i < info->mMipCount; ++i)
                {
                    MipHeader* mipHeader = info->mMipHeaders + i;
                    byte* mipData = info->mImageData + mipHeader->mDataOffset;

                    if (info->mSubImage)
                    {
                        ErrorIf(mipHeader->mLevel != 0, "Sub-image uploading to lower mip levels is not supported.");
                        uint xOffset = info->mXOffset;
                        uint yOffset = info->mHeight - (mipHeader->mHeight + info->mYOffset);
                        glTexSubImage2D(GlTextureFace(static_cast<TextureFace::Enum>(mipHeader->mFace)),
                                        mipHeader->mLevel,
                                        xOffset,
                                        yOffset,
                                        mipHeader->mWidth,
                                        mipHeader->mHeight,
                                        glEnums.mFormat,
                                        glEnums.mType,
                                        mipData);
                    }
                    else
                    {
                        if (info->mCompression != TextureCompression::None)
                            glCompressedTexImage2D(GlTextureFace(static_cast<TextureFace::Enum>(mipHeader->mFace)),
                                                   mipHeader->mLevel,
                                                   GlInternalFormat(info->mCompression),
                                                   mipHeader->mWidth,
                                                   mipHeader->mHeight,
                                                   0,
                                                   mipHeader->mDataSize,
                                                   mipData);
                        else
                            glTexImage2D(GlTextureFace(static_cast<TextureFace::Enum>(mipHeader->mFace)),
                                         mipHeader->mLevel,
                                         glEnums.mInternalFormat,
                                         mipHeader->mWidth,
                                         mipHeader->mHeight,
                                         0,
                                         glEnums.mFormat,
                                         glEnums.mType,
                                         mipData);
                    }
                }
            }

            glTexParameteri(GlTextureType(info->mType), GL_TEXTURE_WRAP_S, GlTextureAddressing(info->mAddressingX));
            glTexParameteri(GlTextureType(info->mType), GL_TEXTURE_WRAP_T, GlTextureAddressing(info->mAddressingY));
            glTexParameteri(GlTextureType(info->mType), GL_TEXTURE_MIN_FILTER, GlTextureFilteringMin(info->mFiltering));
            glTexParameteri(GlTextureType(info->mType), GL_TEXTURE_MAG_FILTER, GlTextureFilteringMag(info->mFiltering));
            glTexParameteri(GlTextureType(info->mType), GL_TEXTURE_COMPARE_MODE, GlCompareMode(info->mCompareMode));
            glTexParameteri(GlTextureType(info->mType), GL_TEXTURE_COMPARE_FUNC, GlCompareFunc(info->mCompareFunc));
            glTexParameterf(GlTextureType(info->mType), GL_TEXTURE_MAX_ANISOTROPY_EXT,
                            GlTextureAnisotropy(info->mAnisotropy));
            glTexParameteri(GlTextureType(info->mType), GL_TEXTURE_MAX_LEVEL, GlTextureMipMapping(info->mMipMapping));

            if (info->mMipMapping == TextureMipMapping::GpuGenerated)
            {
                if (info->mMaxMipOverride > 0)
                    glTexParameteri(GlTextureType(info->mType), GL_TEXTURE_MAX_LEVEL, info->mMaxMipOverride);
                glGenerateMipmap(GlTextureType(info->mType));
            }

            glBindTexture(GlTextureType(info->mType), 0);
        }

        renderData->mType = info->mType;
        renderData->mFormat = info->mFormat;
        renderData->mWidth = info->mWidth;
        renderData->mHeight = info->mHeight;

        renderData->mSamplerSettings = 0;
        renderData->mSamplerSettings |= SamplerSettings::AddressingX(info->mAddressingX);
        renderData->mSamplerSettings |= SamplerSettings::AddressingY(info->mAddressingY);
        renderData->mSamplerSettings |= SamplerSettings::Filtering(info->mFiltering);
        renderData->mSamplerSettings |= SamplerSettings::CompareMode(info->mCompareMode);
        renderData->mSamplerSettings |= SamplerSettings::CompareFunc(info->mCompareFunc);

        delete[] info->mImageData;
        delete[] info->mMipHeaders;
    }

    void OpenglRenderer::RemoveMaterial(MaterialRenderData* data)
    {
        mMaterialRenderDataToDestroy.PushBack(static_cast<GlMaterialRenderData*>(data));
    }

    void OpenglRenderer::RemoveMesh(MeshRenderData* data)
    {
        mMeshRenderDataToDestroy.PushBack(static_cast<GlMeshRenderData*>(data));
    }

    void OpenglRenderer::RemoveTexture(TextureRenderData* data)
    {
        mTextureRenderDataToDestroy.PushBack(static_cast<GlTextureRenderData*>(data));
    }

    bool OpenglRenderer::GetLazyShaderCompilation()
    {
        return mLazyShaderCompilation;
    }

    void OpenglRenderer::SetLazyShaderCompilation(bool isLazy)
    {
        mLazyShaderCompilation = isLazy;
    }

    void OpenglRenderer::AddShaders(Array<ShaderEntry>& entries, uint forceCompileBatchCount)
    {
        if (mLazyShaderCompilation && forceCompileBatchCount == 0)
        {
            forRange(ShaderEntry& entry, entries.All())
            {
                ShaderKey shaderKey(entry.mComposite, StringPair(entry.mCoreVertex, entry.mRenderPass));
                mShaderEntries.Insert(shaderKey, entry);
            }
            entries.Clear();
        }
        else
        {
            uint processCount = Math::Min(forceCompileBatchCount, static_cast<uint>(entries.Size()));
            if (processCount == 0)
                processCount = entries.Size();

            for (uint i = 0; i < processCount; ++i)
            {
                ShaderEntry& entry = entries[i];
                ShaderKey shaderKey(entry.mComposite, StringPair(entry.mCoreVertex, entry.mRenderPass));
                mShaderEntries.Erase(shaderKey);
                CreateShader(entry);
            }

            entries.Erase(entries.SubRange(0, processCount));
        }
    }

    void OpenglRenderer::RemoveShaders(Array<ShaderEntry>& entries)
    {
        forRange(ShaderEntry& entry, entries)
        {
            ShaderKey shaderKey(entry.mComposite, StringPair(entry.mCoreVertex, entry.mRenderPass));

            if (mGlShaders.ContainsKey(shaderKey))
                glDeleteProgram(mGlShaders[shaderKey].mId);

            mGlShaders.Erase(shaderKey);
            mShaderEntries.Erase(shaderKey);
        }
    }

    void OpenglRenderer::SetVSync(bool vsync)
    {
        int swapInterval = vsync ? 1 : 0;
        plGlSetSwapInterval(this, swapInterval);
        mVsync = vsync;
    }

    void OpenglRenderer::GetTextureData(GetTextureDataInfo* info)
    {
        GlTextureRenderData* renderData = static_cast<GlTextureRenderData*>(info->mRenderData);
        info->mImage = nullptr;
        if (!IsColorFormat(renderData->mFormat))
            return;

        info->mWidth = renderData->mWidth;
        info->mHeight = renderData->mHeight;
        if (info->mWidth == 0 || info->mHeight == 0)
            return;

        if (IsFloatColorFormat(renderData->mFormat))
            info->mFormat = TextureFormat::RGB32f;
        else if (IsShortColorFormat(renderData->mFormat))
            info->mFormat = TextureFormat::RGBA16;
        else
            info->mFormat = TextureFormat::RGBA8;

        SetSingleRenderTargets(mSingleTargetFbo, &info->mRenderData, nullptr);

        uint imageSize = info->mWidth * info->mHeight * GetPixelSize(info->mFormat);
        info->mImage = new byte[imageSize];

        GlTextureEnums textureEnums = gTextureEnums[info->mFormat];
        glReadPixels(0, 0, info->mWidth, info->mHeight, textureEnums.mFormat, textureEnums.mType, info->mImage);

        YInvertNonCompressed(info->mImage, info->mWidth, info->mHeight, GetPixelSize(info->mFormat));

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenglRenderer::ShowProgress(ShowProgressInfo* info)
    {
        // Get data off info (we don't technically need to copy this anymore or lock)
        GlTextureRenderData* loadingTexture = static_cast<GlTextureRenderData*>(info->mLoadingTexture);
        GlTextureRenderData* whiteTexture = static_cast<GlTextureRenderData*>(info->mWhiteTexture);
        GlTextureRenderData* splashTexture = static_cast<GlTextureRenderData*>(info->mSplashTexture);
        GlTextureRenderData* fontTexture = static_cast<GlTextureRenderData*>(info->mFontTexture);
        uint logoFrameSize = info->mLogoFrameSize;
        Array<StreamedVertex> progressText = info->mProgressText;
        int progressWidth = info->mProgressWidth;
        float currentPercent = info->mCurrentPercent;
        double time = info->mTimer.Time();
        bool splashMode = info->mSplashMode;
        float alpha = splashMode ? info->mSplashFade : 1.0f;

        IntVec2 size = plGlGetWindowRenderableSize(this);

        Mat4 viewportToNdc;
        viewportToNdc.BuildTransform(Vec3(-1.0f, 1.0f, 0.0f), Mat3::cIdentity,
                                     Vec3(2.0f / size.x, -2.0f / size.y, 1.0f));

       

        Vec3 loadingScale = Vec3(static_cast<float>(loadingTexture->mWidth),
                                 static_cast<float>(loadingTexture->mHeight), 1.0f);
        Vec3 loadingTranslation = Vec3(size.x * 0.5f, size.y * 0.5f, 0.0f);
        Mat4 loadingTransform;
        loadingTransform.BuildTransform(loadingTranslation, Mat3::cIdentity, loadingScale);
        loadingTransform = viewportToNdc * loadingTransform;

        Vec3 progressScale = Vec3(progressWidth * currentPercent, 20.0f, 1.0f);
        Vec3 progressTranslation =
            Vec3((size.x - loadingScale.x + progressScale.x) * 0.5f, (size.y + loadingScale.y) * 0.5f + 300.0f, 0.0f);
        Mat4 progressTransform;
        progressTransform.BuildTransform(progressTranslation, Mat3::cIdentity, progressScale);
        progressTransform = viewportToNdc * progressTransform;

        Vec3 textScale = Vec3(1.0f);
        Vec3 textTranslation = Vec3((size.x - loadingScale.x) * 0.5f, (size.y + loadingScale.y) * 0.5f + 250.0f, 0.0f);
        Mat4 textTransform;
        textTransform.BuildTransform(textTranslation, Mat3::cIdentity, textScale);
        textTransform = viewportToNdc * textTransform;

        Vec3 splashScale = Vec3(static_cast<float>(size.x), static_cast<float>(size.y),
                                1.0f);
        //if (size.x < splashScale.x)
        //    splashScale *= size.x / splashScale.x;
        //if (size.y < splashScale.y)
        //    splashScale *= size.y / splashScale.y;
        Vec3 splashTranslation = Vec3(size.x * 0.5f, size.y * 0.5f, 0.0f);
        Mat4 splashTransform;
        splashTransform.BuildTransform(splashTranslation, Mat3::cIdentity, splashScale);
        splashTransform = viewportToNdc * splashTransform;

        StreamedVertex quadVertices[] = {
            {Vec3(-0.5f, -0.5f, 0.0f), Vec2(0.0f, 0.0f), Vec4(1.0f)},
            {Vec3(-0.5f, 0.5f, 0.0f), Vec2(0.0f, 1.0f), Vec4(1.0f)},
            {Vec3(0.5f, 0.5f, 0.0f), Vec2(1.0f, 1.0f), Vec4(1.0f)},
            {Vec3(0.5f, 0.5f, 0.0f), Vec2(1.0f, 1.0f), Vec4(1.0f)},
            {Vec3(0.5f, -0.5f, 0.0f), Vec2(1.0f, 0.0f), Vec4(1.0f)},
            {Vec3(-0.5f, -0.5f, 0.0f), Vec2(0.0f, 0.0f), Vec4(1.0f)},
        };

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(mLoadingShader);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        GLint textureLoc = glGetUniformLocation(mLoadingShader, "Texture");
        GLint transformLoc = glGetUniformLocation(mLoadingShader, "Transform");
        GLint uvTransformLoc = glGetUniformLocation(mLoadingShader, "UvTransform");

        GLint alphaLoc = glGetUniformLocation(mLoadingShader, "Alpha");
        glUniform1fv(alphaLoc, 1, &alpha);

        GLint textureSlot = 0;
        glActiveTexture(GL_TEXTURE0 + textureSlot);
        glUniform1iv(textureLoc, 1, &textureSlot);

        glUniformMatrix3fv(uvTransformLoc, 1, cTransposeMatrices, Mat3::cIdentity.array);

        if (!splashMode)
        {
            // Loading
            glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, splashTransform.array);
            glBindTexture(GL_TEXTURE_2D, splashTexture->mId);
            mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
            mStreamedVertexBuffer.FlushBuffer(true);
            
            //// Logo
            //glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, splashTransform.array);
            //glBindTexture(GL_TEXTURE_2D, splashTexture->mId);
            //mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
            //mStreamedVertexBuffer.FlushBuffer(true);

            // Progress bar
            glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, progressTransform.array);
            glBindTexture(GL_TEXTURE_2D, whiteTexture->mId);
            mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
            mStreamedVertexBuffer.FlushBuffer(true);

            // Progress text
            if (progressText.Size() > 0)
            {
                glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, textTransform.array);
                glBindTexture(GL_TEXTURE_2D, fontTexture->mId);
                mStreamedVertexBuffer.AddVertices(&progressText[0], progressText.Size(), PrimitiveType::Triangles);
                mStreamedVertexBuffer.FlushBuffer(true);
            }
        }
        else
        {
            // Splash
            glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, splashTransform.array);
            glBindTexture(GL_TEXTURE_2D, splashTexture->mId);
            mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
            mStreamedVertexBuffer.FlushBuffer(true);
        }

        glDisable(GL_BLEND);
        glUseProgram(0);

        // Disable v-sync so we don't wait on frames (mostly for single threaded mode)
        // This could cause tearing, but it's the loading screen.
        plGlSetSwapInterval(this, 0);

        plGlSwapBuffers(this);
        TracyGpuCollect;

        int swapInterval = mVsync ? 1 : 0;
        plGlSetSwapInterval(this, swapInterval);
    }

    GlShader* OpenglRenderer::GetShader(ShaderKey& shaderKey)
    {
        // Check if new shader is pending
        if (mShaderEntries.ContainsKey(shaderKey))
        {
            ShaderEntry& entry = mShaderEntries[shaderKey];

            CreateShader(entry);
            mShaderEntries.Erase(shaderKey);
        }

        return mGlShaders.FindPointer(shaderKey);
    }

    void OpenglRenderer::DoRenderTasks(RenderTasks* renderTasks, RenderQueues* renderQueues)
    {
		ZoneScoped;
        mRenderTasks = renderTasks;
        mRenderQueues = renderQueues;

        forRange(RenderTaskRange& taskRange, mRenderTasks->mRenderTaskRanges.All())
            DoRenderTaskRange(taskRange);

		{
          ZoneScopedN("SwapBuffers");
     	  TracyGpuZone("SwapBuffer")
	      plGlSwapBuffers(this);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
          TracyGpuCollect;
		}

    	{
          ZoneScopedN("DestroyRenderData");
	      DelayedRenderDataDestruction();
          DestroyUnusedSamplers();
	    }
    }

    void OpenglRenderer::DoRenderTaskRange(RenderTaskRange& taskRange)
    {
        mFrameBlock = &mRenderQueues->mFrameBlocks[taskRange.mFrameBlockIndex];
        mViewBlock = &mRenderQueues->mViewBlocks[taskRange.mViewBlockIndex];

        uint taskIndex = taskRange.mTaskIndex;
        for (uint i = 0; i < taskRange.mTaskCount; ++i)
        {
            ErrorIf(taskIndex >= mRenderTasks->mRenderTaskBuffer.mCurrentIndex, "Render task data is not valid.");
            RenderTask* task = (RenderTask*)&mRenderTasks->mRenderTaskBuffer.mRenderTaskData[taskIndex];
        	
            switch (task->mId)
            {
            case RenderTaskType::ClearTarget:
                DoRenderTaskClearTarget(static_cast<RenderTaskClearTarget*>(task));
                taskIndex += sizeof(RenderTaskClearTarget);
                break;

            case RenderTaskType::RenderPass:
                {
                    RenderTaskRenderPass* renderPass = static_cast<RenderTaskRenderPass*>(task);
                    DoRenderTaskRenderPass(renderPass);
                    // RenderPass tasks can have multiple following task entries for sub
                    // RenderGroup settings. Have to index past all sub tasks.
                    taskIndex += sizeof(RenderTaskRenderPass) * (renderPass->mSubRenderGroupCount + 1);
                    i += renderPass->mSubRenderGroupCount;
                }
                break;

            case RenderTaskType::PostProcess:
                DoRenderTaskPostProcess(static_cast<RenderTaskPostProcess*>(task));
                taskIndex += sizeof(RenderTaskPostProcess);
                break;

            case RenderTaskType::BackBufferBlit:
                DoRenderTaskBackBufferBlit(static_cast<RenderTaskBackBufferBlit*>(task));
                taskIndex += sizeof(RenderTaskBackBufferBlit);
                break;

            case RenderTaskType::TextureUpdate:
                DoRenderTaskTextureUpdate(static_cast<RenderTaskTextureUpdate*>(task));
                taskIndex += sizeof(RenderTaskTextureUpdate);
                break;
            case RenderTaskType::ComputePass:
                Warn("Compute: Implementation Unfinied");
                DoRenderTaskCompute(static_cast<RenderTaskCompute*>(task));
                taskIndex += sizeof(RenderTaskCompute);
                break;
            default:
                Error("Render task not implemented.");
                break;
            }
        }
    }

    void OpenglRenderer::DoRenderTaskClearTarget(RenderTaskClearTarget* task)
    {
	    ZoneScoped;
        SetRenderTargets(task->mRenderSettings);

        glStencilMask(task->mStencilWriteMask);
        glDepthMask(true);

        glClearColor(task->mColor.x, task->mColor.y, task->mColor.z, task->mColor.w);
        glClearDepth(task->mDepth);
        glClearStencil(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glStencilMask(0);
        glDepthMask(false);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenglRenderer::DoRenderTaskRenderPass(RenderTaskRenderPass* task)
    {
        ZoneScopedN("RenderTask");
        ZoneText(task->mRenderPassDisplayName.c_str(), sizeof(task->mRenderPassDisplayName));

    	// Create a map of RenderGroup id to task memory index for every sub group
        // entry.
        HashMap<int, size_t> taskIndexMap;
        while (taskIndexMap.Size() < task->mSubRenderGroupCount)
        {
            size_t index = taskIndexMap.Size() + 1;
            RenderTaskRenderPass* subTask = task + index;
            taskIndexMap.InsertOrError(subTask->mRenderGroupIndex, index);
        }

        // Initialize to invalid index so state is set for the first object.
        int currentTaskIndex = -1;

        // All ViewNodes under the base RenderGroup.
        IndexRange viewNodeRange = mViewBlock->mRenderGroupRanges[task->mRenderGroupIndex];

        {
	        ProfileScopeTree(task->mRenderPassDisplayName, "Plasma::ExecuteRendererJob", Color::HotPink)

        	for (uint i = viewNodeRange.start; i < viewNodeRange.end; ++i)
	        {
	            ViewNode& viewNode = mViewBlock->mViewNodes[i];
	            FrameNode& frameNode = mFrameBlock->mFrameNodes[viewNode.mFrameNodeIndex];

	            // Get the index for this object's RenderGroup settings. Always default to
	            // the base task entry.
	            size_t index = taskIndexMap.FindValue(viewNode.mRenderGroupId, 0);

	            // Sub RenderGroups have unique render settings when a different task index
	            // is encountered. Or this is just the first set.
	            if (index != currentTaskIndex)
	            {
	              // Offsets to sub RenderGroup settings or just the base task.
	              RenderTaskRenderPass* subTask = task + index;

	              // Different RenderPass tasks are also made to denote RenderGroups to not
	              // render. Don't change state or render the object.
	              if (subTask->mRender == false)
	                continue;

	              ZoneScopedN("RenderSubTask");
				  ZoneText(subTask->mRenderPassDisplayName.c_str(), sizeof(subTask->mRenderPassDisplayName));
	              ProfileScopeTree(subTask->mRenderPassDisplayName, "RenderTasksUpdate", Color::LightCyan)

	                  currentTaskIndex = index;

	              // Flush potential pending draw call before changing state.
	              mStreamedVertexBuffer.FlushBuffer(true);

	              mViewportSize = IntVec2(subTask->mRenderSettings.mTargetsWidth, subTask->mRenderSettings.mTargetsHeight);

	              mShaderInputsId = subTask->mShaderInputsId;
	              mRenderPassName = subTask->mRenderPassName;

	              SetRenderSettings(subTask->mRenderSettings, mDriverSupport.mMultiTargetBlend);
	              mClipMode = subTask->mRenderSettings.mScissorMode == ScissorMode::Enabled;

	              // For easily resetting blend settings after overriding.
	              mCurrentBlendSettings = subTask->mRenderSettings.mBlendSettings[0];

	              SetRenderTargets(subTask->mRenderSettings);

	              glViewport(0, 0, mViewportSize.x, mViewportSize.y);
	            }

	            // Render the object.
	            switch (frameNode.mRenderingType)
	            {
	            case RenderingType::Static:
	              mStreamedVertexBuffer.FlushBuffer(true);
	              DrawStatic(viewNode, frameNode);
	              break;

	            case RenderingType::Streamed:
	              DrawStreamed(viewNode, frameNode);
	              break;
	            }
	          }
        }
        mStreamedVertexBuffer.FlushBuffer(true);
        mActiveTexture = 0;
        mActiveMaterial = 0;
        mClipMode = false;
        mCurrentClip = Vec4(0, 0, 0, 0);

        SetShader(0);
        SetRenderSettings(RenderSettings(), mDriverSupport.mMultiTargetBlend);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenglRenderer::DoRenderTaskPostProcess(RenderTaskPostProcess* task)
    {
		
    	
        mViewportSize = IntVec2(task->mRenderSettings.mTargetsWidth, task->mRenderSettings.mTargetsHeight);
        if (mViewportSize.x == 0 || mViewportSize.y == 0)
            return;

        GlMaterialRenderData* materialData = static_cast<GlMaterialRenderData*>(task->mMaterialRenderData);
        if (materialData == nullptr && task->mPostProcessName.Empty() == true)
            return;

        String compositeName = materialData ? materialData->mCompositeName : task->mPostProcessName;
        u64 resourceId = materialData ? static_cast<u64>(materialData->mResourceId) : cFragmentShaderInputsId;

        ShaderKey shaderKey(compositeName, StringPair(cPostVertex, String()));
        GlShader* shader = GetShader(shaderKey);
        if (shader == nullptr)
            return;
    	
        {
			ZoneScopedN("ExecuteRenderJob");
            TracyGpuZone("PostProcess");
			ZoneText(task->mDisplayName.c_str(), sizeof(task->mDisplayName));
            ProfileScopeTree(task->mDisplayName, "Plasma::ExecuteRendererJob", Color::Cyan)

	        SetRenderTargets(task->mRenderSettings);
	        SetRenderSettings(task->mRenderSettings, mDriverSupport.mMultiTargetBlend);

	        glViewport(0, 0, mViewportSize.x, mViewportSize.y);

	        SetShader(shader->mId);

	        SetShaderParameters(mFrameBlock, mViewBlock);

	        // Set Material or PostProcess fragment parameters
	        mNextTextureSlot = 0;
	        SetShaderParameters(resourceId, task->mShaderInputsId, mNextTextureSlot);
	        SetShaderParameters(cGlobalShaderInputsId, task->mShaderInputsId, mNextTextureSlot);


	        // draw fullscreen triangle
	        glBindVertexArray(mTriangleArray);
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, static_cast<void*>(nullptr));
        }
        glBindVertexArray(0);

        SetShader(0);
        SetRenderSettings(RenderSettings(), mDriverSupport.mMultiTargetBlend);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenglRenderer::DoRenderTaskBackBufferBlit(RenderTaskBackBufferBlit* task)
    {
		ZoneScoped;
        GlTextureRenderData* renderData = static_cast<GlTextureRenderData*>(task->mColorTarget);
        ScreenViewport viewport = task->mViewport;

        glBindFramebuffer(GL_READ_FRAMEBUFFER, mSingleTargetFbo);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderData->mId, 0);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
        CheckFramebufferStatus();

        mThreadLock.Lock();
        if (mBackBufferSafe)
            glBlitFramebuffer(0,
                              0,
                              renderData->mWidth,
                              renderData->mHeight,
                              viewport.x,
                              viewport.y,
                              viewport.x + viewport.width,
                              viewport.y + viewport.height,
                              GL_COLOR_BUFFER_BIT,
                              GL_NEAREST);
        mThreadLock.Unlock();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }

    void OpenglRenderer::DoRenderTaskTextureUpdate(RenderTaskTextureUpdate* task)
    {
	    ZoneScoped;
        AddTextureInfo info;
        info.mRenderData = task->mRenderData;
        info.mWidth = task->mWidth;
        info.mDepth = task->mDepth;
        info.mHeight = task->mHeight;
        info.mType = task->mType;
        info.mFormat = task->mFormat;
        info.mAddressingX = task->mAddressingX;
        info.mAddressingY = task->mAddressingY;
        info.mFiltering = task->mFiltering;
        info.mCompareMode = task->mCompareMode;
        info.mCompareFunc = task->mCompareFunc;
        info.mAnisotropy = task->mAnisotropy;
        info.mMipMapping = task->mMipMapping;

        info.mMipCount = 0;
        info.mTotalDataSize = 0;
        info.mImageData = nullptr;
        info.mMipHeaders = nullptr;

        AddTexture(&info);
    }

    void OpenglRenderer::DoRenderTaskCompute(RenderTaskCompute* task)
    {
        GlMaterialRenderData* materialData = static_cast<GlMaterialRenderData*>(task->mMaterialRenderData);
        if (materialData == nullptr && task->mComputePassName.Empty() == true)
            return;

        String compositeName = materialData ? materialData->mCompositeName : task->mComputePassName;

        ShaderKey shaderKey(compositeName, StringPair(cPostVertex, String()));
        GlShader* shader = GetShader(shaderKey);
        if (shader == nullptr)
            return;
        {
            ZoneScopedN("DoRenderTask");
            ProfileScopeTree(task->mComputeDisplayName, "Plasma::DoRenderTaskCompute", Color::Cyan)

            SetShader(shader->mId);

            glDispatchCompute(task->mDispatchSize.x, task->mDispatchSize.y, task->mDispatchSize.z);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
            SetShader(0);
        }

    }

    void OpenglRenderer::SetRenderTargets(RenderSettings& renderSettings)
    {
        if (renderSettings.mSingleColorTarget)
            SetSingleRenderTargets(mSingleTargetFbo, renderSettings.mColorTargets, renderSettings.mDepthTarget);
        else
            SetMultiRenderTargets(mMultiTargetFbo, renderSettings.mColorTargets, renderSettings.mDepthTarget);
    }

    void OpenglRenderer::DrawStatic(ViewNode& viewNode, FrameNode& frameNode)
    {
	    ZoneScoped;
        GlMeshRenderData* meshData = static_cast<GlMeshRenderData*>(frameNode.mMeshRenderData);
        GlMaterialRenderData* materialData = static_cast<GlMaterialRenderData*>(frameNode.mMaterialRenderData);
        if (meshData == nullptr || materialData == nullptr)
            return;

        // Shader permutation lookup for vertex type and render pass
        ShaderKey shaderKey(materialData->mCompositeName,
                            StringPair(GetCoreVertexFragmentName(frameNode.mCoreVertexType), mRenderPassName));
        GlShader* shader = GetShader(shaderKey);
        if (shader == nullptr)
            return;

        if (shader->mId != mActiveShader)
        {
            SetShader(shader->mId);
            // Set non-object built-in inputs once per active shader
            SetShaderParameters(mFrameBlock, mViewBlock);
            mActiveMaterial = 0;
        }

        // Per object built-in inputs
        SetShaderParameters(&frameNode, &viewNode);

        // Set RenderPass inputs once on new shader or if a reset is triggered
        if (mActiveMaterial == 0)
        {
            mNextTextureSlot = 0;
            SetShaderParameters(cFragmentShaderInputsId, mShaderInputsId, mNextTextureSlot);
        }

        // On change of materials, material inputs followed by global inputs have to
        // be reset
        if (materialData->mResourceId != mActiveMaterial)
        {
            mNextTextureSlotMaterial = mNextTextureSlot;
            SetShaderParameters(static_cast<u64>(materialData->mResourceId), mShaderInputsId, mNextTextureSlotMaterial);
            SetShaderParameters(cGlobalShaderInputsId, mShaderInputsId, mNextTextureSlotMaterial);

            mActiveMaterial = static_cast<u64>(materialData->mResourceId);
        }

        // Don't need to use a permanent texture slot
        uint textureSlot = mNextTextureSlotMaterial;

        // Per object shader inputs
        if (frameNode.mShaderInputRange.Count() != 0)
        {
            SetShaderParameters(frameNode.mShaderInputRange, textureSlot);
            // Since object input overrides could apply to other fragments that aren't
            // from the material (i.e. RenderPass) have to trigger a reset of all
            // inputs, done by resetting active material, but shader does not have to be
            // reset
            mActiveMaterial = 0;
        }

        GlTextureRenderData* textureData = static_cast<GlTextureRenderData*>(frameNode.mTextureRenderData);
        if (textureData != nullptr)
        {
            BindTexture(TextureType::Texture2D, textureSlot, textureData->mId, mDriverSupport.mSamplerObjects);
            SetShaderParameter(ShaderInputType::Texture, "HeightMapWeights_HeightMapPBRMap", &textureSlot);
        }

    	TracyGpuZone("DrawStatic");
    	
        glBindVertexArray(meshData->mVertexArray);
        if (meshData->mIndexBuffer == 0)
            // If nothing is bound, glDrawArrays will invoke the shader pipeline the
            // given number of times
            glDrawArrays(GlPrimitiveType(meshData->mPrimitiveType), 0, meshData->mIndexCount);
        else
            glDrawElements(GlPrimitiveType(meshData->mPrimitiveType), meshData->mIndexCount, GL_UNSIGNED_INT,
                           static_cast<void*>(nullptr));
        glBindVertexArray(0);
    }

    void OpenglRenderer::DrawStreamed(ViewNode& viewNode, FrameNode& frameNode)
    {
        ZoneScoped;
        GlMaterialRenderData* materialData = static_cast<GlMaterialRenderData*>(frameNode.mMaterialRenderData);
        if (materialData == nullptr)
            return;

        // Shader permutation lookup for vertex type and render pass
        ShaderKey shaderKey(materialData->mCompositeName,
                            StringPair(GetCoreVertexFragmentName(frameNode.mCoreVertexType), mRenderPassName));
        GlShader* shader = GetShader(shaderKey);
        if (shader == nullptr)
            return;

        if (viewNode.mStreamedVertexCount == 0)
            return;

        u64 materialId = materialData->mResourceId;
        GLuint shaderId = shader->mId;
        GLuint textureId = 0;

        GlTextureRenderData* textureData = static_cast<GlTextureRenderData*>(frameNode.mTextureRenderData);
        if (textureData != nullptr)
            textureId = textureData->mId;

        if (mCurrentLineWidth != frameNode.mBorderThickness)
        {
            mCurrentLineWidth = frameNode.mBorderThickness;
            mStreamedVertexBuffer.FlushBuffer(false);
            glLineWidth(frameNode.mBorderThickness);
        }

        if (mClipMode && frameNode.mClip != mCurrentClip)
        {
            mStreamedVertexBuffer.FlushBuffer(false);
            mCurrentClip = frameNode.mClip;
            glScissor(static_cast<int>(mCurrentClip.x),
                      mViewportSize.y - static_cast<int>(mCurrentClip.y) - static_cast<int>(mCurrentClip.w),
                      static_cast<int>(mCurrentClip.z),
                      static_cast<int>(mCurrentClip.w));
        }

        // Check for any state change
        if (shaderId != mActiveShader || textureId != mActiveTexture || materialId != mActiveMaterial)
        {
            mStreamedVertexBuffer.FlushBuffer(false);

            SetShader(shaderId);
            mActiveTexture = textureId;
            mActiveMaterial = materialId;

            // Set non-object data once per active shader
            SetShaderParameters(mFrameBlock, mViewBlock);
            // Set RenderPass fragment parameters
            mNextTextureSlot = 0;
            SetShaderParameters(cFragmentShaderInputsId, mShaderInputsId, mNextTextureSlot);

            SetShaderParameters(static_cast<u64>(materialData->mResourceId), mShaderInputsId, mNextTextureSlot);
            SetShaderParameters(cGlobalShaderInputsId, mShaderInputsId, mNextTextureSlot);

            if (textureId != 0)
            {
                BindTexture(textureData->mType, mNextTextureSlot, textureId, mDriverSupport.mSamplerObjects);
                if (textureData->mType == TextureType::TextureCube)
                    SetShaderParameter(ShaderInputType::Texture, cSpriteSourceCubePreview, &mNextTextureSlot);
                else
                    SetShaderParameter(ShaderInputType::Texture, cSpriteSource, &mNextTextureSlot);
                ++mNextTextureSlot;
            }
        }

        // Have to force an independent draw call if object has individual shader
        // inputs
        if (frameNode.mShaderInputRange.Count() != 0)
        {
            mStreamedVertexBuffer.FlushBuffer(false);
            SetShaderParameters(frameNode.mShaderInputRange, mNextTextureSlot);
            mActiveMaterial = 0;
        }

        if (frameNode.mBlendSettingsOverride)
        {
            // Only overrides for target 0, temporary functionality for viewports
            mStreamedVertexBuffer.FlushBuffer(false);
            SetBlendSettings(mRenderQueues->mBlendSettingsOverrides[frameNode.mBlendSettingsIndex]);
        }

        uint vertexStart = viewNode.mStreamedVertexStart;
        uint vertexCount = viewNode.mStreamedVertexCount;
        mStreamedVertexBuffer.AddVertices(
            mRenderQueues->mStreamedVertices, vertexStart, vertexCount, viewNode.mStreamedVertexType);

        if (frameNode.mBlendSettingsOverride)
        {
            mStreamedVertexBuffer.FlushBuffer(false);
            SetBlendSettings(mCurrentBlendSettings);
        }
    }

    void OpenglRenderer::SetShaderParameter(ShaderInputType::Enum uniformType, StringParam name, void* data)
    {
        GLint location = glGetUniformLocation(mActiveShader, name.c_str());
        if (location == -1)
            return;
        mUniformFunctions[uniformType](location, 1, data);
    }

    void OpenglRenderer::SetShaderParameterMatrix(StringParam name, Mat3& transform)
    {
        GLint location = glGetUniformLocation(mActiveShader, name.c_str());
        if (location == -1)
            return;
        glUniformMatrix3fv(location, 1, cTransposeMatrices, transform.array);
    }

    void OpenglRenderer::SetShaderParameterMatrix(StringParam name, Mat4& transform)
    {
        GLint location = glGetUniformLocation(mActiveShader, name.c_str());
        if (location == -1)
            return;
        glUniformMatrix4fv(location, 1, cTransposeMatrices, transform.array);
    }

    void OpenglRenderer::SetShaderParameterMatrixInv(StringParam name, Mat3& transform)
    {
        GLint location = glGetUniformLocation(mActiveShader, name.c_str());
        if (location == -1)
            return;
        Mat3 inverse = transform.Inverted();
        glUniformMatrix3fv(location, 1, cTransposeMatrices, inverse.array);
    }

    void OpenglRenderer::SetShaderParameterMatrixInv(StringParam name, Mat4& transform)
    {
        GLint location = glGetUniformLocation(mActiveShader, name.c_str());
        if (location == -1)
            return;

        Mat4 inverse = transform.Inverted();
        glUniformMatrix4fv(location, 1, cTransposeMatrices, inverse.array);
    }

    void OpenglRenderer::SetShaderParameters(FrameBlock* frameBlock, ViewBlock* viewBlock)
    {
        SetShaderParameter(ShaderInputType::Float, cFrameTime, &frameBlock->mFrameTime);
        SetShaderParameter(ShaderInputType::Float, cLogicTime, &frameBlock->mLogicTime);

        SetShaderParameterMatrix(cWorldToView, viewBlock->mWorldToView);
        SetShaderParameterMatrix(cViewToPerspective, viewBlock->mViewToPerspective);
        SetShaderParameterMatrix(cPlasmaPerspectiveToApiPerspective, viewBlock->mPlasmaPerspectiveToApiPerspective);
        SetShaderParameterMatrixInv(cViewToWorld, viewBlock->mWorldToView);
        SetShaderParameterMatrixInv(cPerspectiveToView, viewBlock->mViewToPerspective);

        SetShaderParameter(ShaderInputType::Float, cNearPlane, &viewBlock->mNearPlane);
        SetShaderParameter(ShaderInputType::Float, cFarPlane, &viewBlock->mFarPlane);
        SetShaderParameter(ShaderInputType::Vec2, cViewportSize, viewBlock->mViewportSize.array);
        SetShaderParameter(ShaderInputType::Vec2, cInverseViewportSize, viewBlock->mInverseViewportSize.array);
    }

    void OpenglRenderer::SetShaderParameters(FrameNode* frameNode, ViewNode* viewNode)
    {
        SetShaderParameterMatrix(cLocalToWorld, frameNode->mLocalToWorld);
        SetShaderParameterMatrix(cLocalToWorldNormal, frameNode->mLocalToWorldNormal);
        SetShaderParameterMatrixInv(cWorldToLocal, frameNode->mLocalToWorld);
        SetShaderParameterMatrixInv(cWorldToLocalNormal, frameNode->mLocalToWorldNormal);

        SetShaderParameter(ShaderInputType::Vec3, cObjectWorldPosition, frameNode->mObjectWorldPosition.array);

        uint boneCount = frameNode->mBoneMatrixRange.Count();
        uint remapCount = frameNode->mIndexRemapRange.Count();
        if (boneCount > 0 && remapCount > 0)
        {
            GlMeshRenderData* meshData = static_cast<GlMeshRenderData*>(frameNode->mMeshRenderData);

            Array<Mat4> remappedBoneTransforms;
            for (uint i = frameNode->mIndexRemapRange.start; i < frameNode->mIndexRemapRange.end; ++i)
            {
                uint meshIndex = i - frameNode->mIndexRemapRange.start;
                uint bufferIndex = mRenderQueues->mIndexRemapBuffer[i] + frameNode->mBoneMatrixRange.start;

                remappedBoneTransforms.PushBack(mRenderQueues->mSkinningBuffer[bufferIndex] *
                    meshData->mBones[meshIndex].mBindTransform);
            }

            GLint location = glGetUniformLocation(mActiveShader, "MiscData.BoneTransforms");
            if (location != -1)
            {
                glUniformMatrix4fv(location, remappedBoneTransforms.Size(), cTransposeMatrices,
                                   remappedBoneTransforms[0].array);
            }
        }

        SetShaderParameterMatrix(cLocalToView, viewNode->mLocalToView);
        SetShaderParameterMatrix(cLastLocalToView, viewNode->mLastLocalToView);
        SetShaderParameterMatrix(cLocalToViewNormal, viewNode->mLocalToViewNormal);
        SetShaderParameterMatrix(cLocalToPerspective, viewNode->mLocalToPerspective);
        SetShaderParameterMatrixInv(cViewToLocal, viewNode->mLocalToView);
        SetShaderParameterMatrixInv(cViewToLocalNormal, viewNode->mLocalToViewNormal);
    }

    void OpenglRenderer::SetShaderParameters(IndexRange inputRange, uint& nextTextureSlot)
    {
        Array<ShaderInput>::range shaderInputs = mRenderTasks->mShaderInputs.SubRange(inputRange.start,
                                                                                      inputRange.Count());

        forRange(ShaderInput& input, shaderInputs)
        {
            if (input.mShaderInputType == ShaderInputType::Texture)
            {
                GlTextureRenderData* textureData = *(GlTextureRenderData**)input.mValue;
                BindTexture(textureData->mType, nextTextureSlot, textureData->mId, mDriverSupport.mSamplerObjects);

                // Check for custom sampler settings for this input
                // If any sampler attributes were set on the shader input then
                // mSamplerSettings will be non-plasma If driver does not have sampler
                // object support then this feature does nothing
                if (input.mSamplerSettings != 0 && mDriverSupport.mSamplerObjects)
                {
                    u32 samplerSettings = input.mSamplerSettings;
                    // Use texture settings as defaults so that only attributes specified on
                    // shader input differ from the texture
                    SamplerSettings::FillDefaults(samplerSettings, textureData->mSamplerSettings);
                    GLuint sampler = GetSampler(samplerSettings);
                    glBindSampler(nextTextureSlot, sampler);
                }

                SetShaderParameter(input.mShaderInputType, input.mTranslatedInputName, &nextTextureSlot);
                ++nextTextureSlot;
            }
            else if (input.mShaderInputType == ShaderInputType::Bool)
            {
                int value = *(bool*)input.mValue;
                SetShaderParameter(input.mShaderInputType, input.mTranslatedInputName, &value);
            }
            else if (input.mShaderInputType == ShaderInputType::Mat3)
            {
                SetShaderParameterMatrix(input.mTranslatedInputName, *(Mat3*)input.mValue);
            }
            else if (input.mShaderInputType == ShaderInputType::Mat4)
            {
                SetShaderParameterMatrix(input.mTranslatedInputName, *(Mat4*)input.mValue);
            }
            else
            {
                SetShaderParameter(input.mShaderInputType, input.mTranslatedInputName, input.mValue);
            }
        }
    }

    void OpenglRenderer::SetShaderParameters(u64 objectId, uint shaderInputsId, uint& nextTextureSlot)
    {
        Pair<u64, uint> pair(objectId, shaderInputsId);
        IndexRange inputRange = mRenderTasks->mShaderInputRanges.FindValue(pair, IndexRange(0, 0));
        SetShaderParameters(inputRange, nextTextureSlot);
    }

    void OpenglRenderer::CreateShader(ShaderEntry& entry)
    {
#ifdef PlasmaDebug
  PlasmaPrint(
      "Compiling shader: %s %s %s\n", entry.mCoreVertex.c_str(), entry.mComposite.c_str(), entry.mRenderPass.c_str());
#endif

        ShaderKey shaderKey(entry.mComposite, StringPair(entry.mCoreVertex, entry.mRenderPass));

        GLuint shaderId = 0;
        CreateShader(entry.mVertexShader, entry.mGeometryShader, entry.mPixelShader, shaderId);

        // Shouldn't fail at this point. Not currently handling gl errors.
        ErrorIf(shaderId == 0, "Failed to compile or link shader.");

        GlShader shader;
        shader.mId = shaderId;

        // Must delete old shader after new one is created or something is getting
        // incorrectly cached/generated
        if (mGlShaders.ContainsKey(shaderKey))
            glDeleteProgram(mGlShaders[shaderKey].mId);

        mGlShaders.Insert(shaderKey, shader);
    }

    void OpenglRenderer::CreateShader(StringParam vertexSource,
                                      StringParam geometrySource,
                                      StringParam pixelSource,
                                      GLuint& shader)
    {
#ifdef PlasmaDebug
  Timer compileTimer;
#endif

        GLuint program = glCreateProgram();

        const GLchar* vertexSourceData = vertexSource.Data();
        GLint vertexSourceSize = vertexSource.SizeInBytes();
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glAttachShader(program, vertexShader);
        glShaderSource(vertexShader, 1, &vertexSourceData, &vertexSourceSize);
        glCompileShader(vertexShader);
        CheckShader(vertexShader, vertexSource);

        GLuint geometryShader = 0;
        if (!geometrySource.Empty())
        {
            const GLchar* geometrySourceData = geometrySource.Data();
            GLint geometrySourceSize = geometrySource.SizeInBytes();
            geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
            glAttachShader(program, geometryShader);
            glShaderSource(geometryShader, 1, &geometrySourceData, &geometrySourceSize);
            glCompileShader(geometryShader);
            CheckShader(geometryShader, geometrySource);
        }

        const GLchar* pixelSourceData = pixelSource.Data();
        GLint pixelSourceSize = pixelSource.SizeInBytes();
        GLuint pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
        glAttachShader(program, pixelShader);
        glShaderSource(pixelShader, 1, &pixelSourceData, &pixelSourceSize);
        glCompileShader(pixelShader);
        CheckShader(pixelShader, pixelSource);

        glBindAttribLocation(program, VertexSemantic::Position, "LocalPosition");
        glBindAttribLocation(program, VertexSemantic::Normal, "LocalNormal");
        glBindAttribLocation(program, VertexSemantic::Tangent, "LocalTangent");
        glBindAttribLocation(program, VertexSemantic::Bitangent, "LocalBitangent");
        glBindAttribLocation(program, VertexSemantic::Uv, "Uv");
        glBindAttribLocation(program, VertexSemantic::UvAux, "UvAux");
        glBindAttribLocation(program, VertexSemantic::Color, "Color");
        glBindAttribLocation(program, VertexSemantic::ColorAux, "ColorAux");
        glBindAttribLocation(program, VertexSemantic::BoneIndices, "BoneIndices");
        glBindAttribLocation(program, VertexSemantic::BoneWeights, "BoneWeights");
        // Not implemented by geometry processor
        glBindAttribLocation(program, 10, "Aux0");
        glBindAttribLocation(program, 11, "Aux1");
        glBindAttribLocation(program, 12, "Aux2");
        glBindAttribLocation(program, 13, "Aux3");
        glBindAttribLocation(program, 14, "Aux4");
        glBindAttribLocation(program, 15, "Aux5");

#ifdef PlasmaDebug
  double compileSeconds = compileTimer.UpdateAndGetTime();
  PlasmaPrint("Compiled shader in %f seconds\n", compileSeconds);
#endif

#ifdef PlasmaDebug
  Timer linkTimer;
#endif

        glLinkProgram(program);

#ifdef PlasmaDebug
  double linkSeconds = linkTimer.UpdateAndGetTime();
  PlasmaPrint("Linked shader in %f seconds\n", linkSeconds);
#endif

#ifdef PlasmaDebug
  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE)
  {
    GLint infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar* strInfoLog = (GLchar*)alloca(infoLogLength + 1);
    glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
    PlasmaPrint("Link Error\n%s\n", strInfoLog);

    static size_t sMaxPrints = 4;
    if (sMaxPrints > 0)
    {
      PlasmaPrint("\n************************************************************"
             "VERTEX\n%s"
             "\n************************************************************"
             "GEOMETRY\n%s"
             "\n************************************************************"
             "PIXEL\n%s"
             "\n************************************************************\n",
             vertexSource.c_str(),
             geometrySource.c_str(),
             pixelSource.c_str());
      --sMaxPrints;
    }
  }
#endif

        // For now we always output the shader assuming that all of them compile and
        // link. This is because requesting the link status is a blocking operation.
        shader = program;

        glDetachShader(program, vertexShader);
        glDetachShader(program, pixelShader);
        glDeleteShader(vertexShader);
        glDeleteShader(pixelShader);

        if (geometryShader != 0)
        {
            glDetachShader(program, geometryShader);
            glDeleteShader(geometryShader);
        }

        // We don't currently do this because we don't want to check the status of the
        // shader (blocking).
        // if (status == GL_FALSE)
        //  glDeleteProgram(program);
    }

    void OpenglRenderer::SetShader(GLuint shader)
    {
        mActiveShader = shader;
        glUseProgram(mActiveShader);
    }

    void OpenglRenderer::DelayedRenderDataDestruction()
    {
        forRange(GlMaterialRenderData* renderData, mMaterialRenderDataToDestroy.All())
            DestroyRenderData(renderData);
        forRange(GlMeshRenderData* renderData, mMeshRenderDataToDestroy.All())
            DestroyRenderData(renderData);
        forRange(GlTextureRenderData* renderData, mTextureRenderDataToDestroy.All())
            DestroyRenderData(renderData);

        mMaterialRenderDataToDestroy.Clear();
        mMeshRenderDataToDestroy.Clear();
        mTextureRenderDataToDestroy.Clear();
    }

    void OpenglRenderer::DestroyRenderData(GlMaterialRenderData* renderData)
    {
        delete renderData;
    }

    void OpenglRenderer::DestroyRenderData(GlMeshRenderData* renderData)
    {
        GlMeshRenderData* glRenderData = static_cast<GlMeshRenderData*>(renderData);
        glDeleteVertexArrays(1, &glRenderData->mVertexArray);
        glDeleteBuffers(1, &glRenderData->mVertexBuffer);
        glDeleteBuffers(1, &glRenderData->mIndexBuffer);

        delete renderData;
    }

    void OpenglRenderer::DestroyRenderData(GlTextureRenderData* renderData)
    {
        GlTextureRenderData* glRenderData = static_cast<GlTextureRenderData*>(renderData);
        glDeleteTextures(1, &glRenderData->mId);
        delete renderData;
    }

    GLuint OpenglRenderer::GetSampler(u32 samplerSettings)
    {
        // Sampler objects can be reused for any number of texture units
        if (mSamplers.ContainsKey(samplerSettings))
        {
            mUnusedSamplers.Erase(samplerSettings);
            return mSamplers[samplerSettings];
        }

        GLuint newSampler;
        glGenSamplers(1, &newSampler);

        TextureAddressing::Enum addressingX = SamplerSettings::AddressingX(samplerSettings);
        TextureAddressing::Enum addressingY = SamplerSettings::AddressingY(samplerSettings);
        TextureFiltering::Enum filtering = SamplerSettings::Filtering(samplerSettings);
        TextureCompareMode::Enum compareMode = SamplerSettings::CompareMode(samplerSettings);
        TextureCompareFunc::Enum compareFunc = SamplerSettings::CompareFunc(samplerSettings);

        glSamplerParameteri(newSampler, GL_TEXTURE_WRAP_S, GlTextureAddressing(addressingX));
        glSamplerParameteri(newSampler, GL_TEXTURE_WRAP_T, GlTextureAddressing(addressingY));
        glSamplerParameteri(newSampler, GL_TEXTURE_MIN_FILTER, GlTextureFilteringMin(filtering));
        glSamplerParameteri(newSampler, GL_TEXTURE_MAG_FILTER, GlTextureFilteringMag(filtering));
        glSamplerParameteri(newSampler, GL_TEXTURE_COMPARE_MODE, GlCompareMode(compareMode));
        glSamplerParameteri(newSampler, GL_TEXTURE_COMPARE_FUNC, GlCompareFunc(compareFunc));

        mSamplers.Insert(samplerSettings, newSampler);

        return newSampler;
    }

    void OpenglRenderer::DestroyUnusedSamplers()
    {
        forRange(u32 id, mUnusedSamplers.All())
        {
            GLuint sampler = mSamplers[id];
            glDeleteSamplers(1, &sampler);
            mSamplers.Erase(id);
        }
        mUnusedSamplers.Clear();
        mUnusedSamplers.Append(mSamplers.Keys());
    }
} // namespace Plasma

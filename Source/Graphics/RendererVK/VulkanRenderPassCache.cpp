#include "Precompiled.hpp"

namespace Plasma
{
    void RenderPassCacheEntry::Load(const VulkanRenderPassInfo& renderPassInfo)
    {
        auto copyAttachmentInfo = [](AttachmentDescription& dst, const VulkanRenderPassInfo::AttachmentDescription& src, const VulkanImageView* imageView)
        {
            dst.mFormat = imageView->GetImageFormat();
            dst.mSampleCount = VK_SAMPLE_COUNT_1_BIT;
            dst.mLoadOp = src.mLoadOp;
            dst.mStoreOp = src.mStoreOp;
            dst.mStencilLoadOp = src.mStencilLoadOp;
            dst.mStencilStoreOp = src.mStencilStoreOp;
            dst.mInitialLayout = src.mInitialLayout;
            dst.mFinalLayout = src.mFinalLayout;
        };

        mBaseLayer = renderPassInfo.mBaseLayer;
        mNumLayers = renderPassInfo.mNumLayers;
        mColorAttachmentCount = renderPassInfo.mColorAttachmentCount;
        for (size_t i = 0; i < mColorAttachmentCount; ++i)
            copyAttachmentInfo(mColorDescriptions[i], renderPassInfo.mColorDescriptions[i], renderPassInfo.mColorAttachments[i]);
        copyAttachmentInfo(mDepthDescription, renderPassInfo.mDepthDescription, renderPassInfo.mDepthAttachment);

        mSubPasses.Resize(renderPassInfo.mSubPasses.Size());
        for (size_t subPassIndex = 0; subPassIndex < mSubPasses.Size(); ++subPassIndex)
        {
            SubPass& dstSubPass = mSubPasses[subPassIndex];
            const VulkanRenderPassInfo::SubPass& srcSubPass = renderPassInfo.mSubPasses[subPassIndex];
            dstSubPass.mColorAttachmentCount = srcSubPass.mColorAttachmentCount;
            for (size_t attachmentIndex = 0; attachmentIndex < dstSubPass.mColorAttachmentCount; ++attachmentIndex)
                dstSubPass.mColorAttachments[attachmentIndex] = srcSubPass.mColorAttachments[attachmentIndex];
        }
    }

    size_t RenderPassCacheEntry::Hash(bool compatible) const
    {
        auto attachmentHashFn = [compatible](Hasher& hasher, const AttachmentDescription& description)
        {
            hasher.U32(description.mFormat);
            hasher.U32(description.mSampleCount);
            if (!compatible)
            {
                hasher.U32(description.mLoadOp);
                hasher.U32(description.mStoreOp);
                hasher.U32(description.mStencilLoadOp);
                hasher.U32(description.mStencilStoreOp);
                hasher.U32(description.mInitialLayout);
                hasher.U32(description.mFinalLayout);
            }
        };

        Hasher hasher;
        hasher.U32(mBaseLayer);
        hasher.U32(mNumLayers);
        hasher.U32(mColorAttachmentCount);
        for (byte i = 0; i < mColorAttachmentCount; ++i)
            attachmentHashFn(hasher, mColorDescriptions[i]);
        attachmentHashFn(hasher, mDepthDescription);

        for (size_t subPassIndex = 0; subPassIndex < mSubPasses.Size(); ++subPassIndex)
        {
            const SubPass& subPass = mSubPasses[subPassIndex];
            hasher.U32(subPass.mColorAttachmentCount);
            for (size_t attachmentIndex = 0; attachmentIndex < subPass.mColorAttachmentCount; ++attachmentIndex)
                hasher.U32(subPass.mColorAttachments[attachmentIndex]);
        }

        return hasher.mHash;
    }

    bool RenderPassCacheEntry::Equal(const RenderPassCacheEntry& rhs, bool compatible) const
    {
        auto attachmentEqualFn = [compatible](const AttachmentDescription& lhs, const AttachmentDescription& rhs)
        {
            bool equal = true;
            equal &= (lhs.mFormat == rhs.mFormat);
            equal &= (lhs.mSampleCount == rhs.mSampleCount);
            if (!compatible)
            {
                equal &= (lhs.mLoadOp == rhs.mLoadOp);
                equal &= (lhs.mStoreOp == rhs.mStoreOp);
                equal &= (lhs.mStencilLoadOp == rhs.mStencilLoadOp);
                equal &= (lhs.mStencilStoreOp == rhs.mStencilStoreOp);
                equal &= (lhs.mInitialLayout == rhs.mInitialLayout);
                equal &= (lhs.mFinalLayout == rhs.mFinalLayout);
            }
            return equal;
        };

        if (mColorAttachmentCount != rhs.mColorAttachmentCount)
            return false;
        if (mSubPasses.Size() != rhs.mSubPasses.Size())
            return false;

        bool equal = true;
        equal &= (mBaseLayer == rhs.mBaseLayer);
        equal &= (mNumLayers == rhs.mNumLayers);
        for (byte i = 0; i < mColorAttachmentCount; ++i)
            equal &= attachmentEqualFn(mColorDescriptions[i], rhs.mColorDescriptions[i]);
        equal &= attachmentEqualFn(mDepthDescription, rhs.mDepthDescription);

        for (size_t subPassIndex = 0; subPassIndex < mSubPasses.Size(); ++subPassIndex)
        {
            const SubPass& lhsSubPass = mSubPasses[subPassIndex];
            const SubPass& rhsSubPass = rhs.mSubPasses[subPassIndex];
            if (lhsSubPass.mColorAttachmentCount != rhsSubPass.mColorAttachmentCount)
                return false;
            for (size_t attachmentIndex = 0; attachmentIndex < lhsSubPass.mColorAttachmentCount; ++attachmentIndex)
                equal &= (lhsSubPass.mColorAttachments[attachmentIndex] == rhsSubPass.mColorAttachments[attachmentIndex]);
        }

        return equal;
    }

    //-------------------------------------------------------------------RenderPassCache
    RenderPassCache::RenderPassCache(VkDevice device)
        : mDevice(device)
    {
    }

    RenderPassCache::~RenderPassCache()
    {
        Free();
    }

    VulkanRenderPass* RenderPassCache::FindOrCreate(const VulkanRenderPassInfo& renderPassInfo)
    {
        RenderPassCacheEntry entry;
        entry.Load(renderPassInfo);
        VulkanRenderPass* renderPass = mRenderPasses.FindValue(entry, nullptr);
        if (renderPass == nullptr)
            renderPass = Create(renderPassInfo, entry);
        return renderPass;
    }

    VulkanRenderPass* RenderPassCache::FindOrCreateCompatible(const VulkanRenderPassInfo& renderPassInfo)
    {
        RenderPassCacheEntry entry;
        entry.Load(renderPassInfo);
        VulkanRenderPass* renderPass = mCompatibleRenderPasses.FindValue(entry, nullptr);
        if (renderPass == nullptr)
            renderPass = Create(renderPassInfo, entry);
        return renderPass;
    }

    VulkanRenderPass* RenderPassCache::Find(const RenderPassCookie& cookie)
    {
        return mRenderPassCookieMap.FindValue(cookie, nullptr);
    }

    VulkanRenderPass* RenderPassCache::FindCompatible(const RenderPassCookie& cookie)
    {
        RenderPassCookie* compatibleCookie = mCompatibleRenderPassCookieMap.FindPointer(cookie);
        if (compatibleCookie == nullptr)
            return nullptr;

        return mRenderPassCookieMap.FindValue(*compatibleCookie, nullptr);
    }

    void RenderPassCache::Free()
    {
        for (VulkanRenderPass* renderPass : mRenderPasses.Values())
            delete renderPass;

        mRenderPasses.Clear();
        mCompatibleRenderPasses.Clear();
    }

    VulkanRenderPass* RenderPassCache::Create(const VulkanRenderPassInfo& renderPassInfo, const RenderPassCacheEntry& cacheEntry)
    {
        VulkanRenderPass* renderPass = new VulkanRenderPass(mDevice, renderPassInfo);
        RenderPassCookie cookie = *renderPass;
        mRenderPasses.InsertOrError(cacheEntry, renderPass);
        mRenderPassCookieMap.InsertOrError(cookie, renderPass);

        VulkanRenderPass* compatibleRenderPass = mCompatibleRenderPasses.FindValue(cacheEntry, nullptr);
        if (compatibleRenderPass == nullptr)
        {
            mCompatibleRenderPasses.InsertNoOverwrite(cacheEntry, renderPass);
            mCompatibleRenderPassCookieMap.InsertOrError(cookie, cookie);
        }
        else
        {
            RenderPassCookie compatibleCookie = *compatibleRenderPass;
            mCompatibleRenderPassCookieMap.InsertOrError(cookie, compatibleCookie);
        }

        return renderPass;
    }

}
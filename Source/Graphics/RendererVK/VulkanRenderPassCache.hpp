#pragma once

namespace Plasma
{
    //-------------------------------------------------------------------RenderPassCacheEntry
    struct RenderPassCacheEntry
    {
        struct AttachmentDescription
        {
            VkFormat mFormat = VK_FORMAT_UNDEFINED;
            VkSampleCountFlagBits mSampleCount = VK_SAMPLE_COUNT_1_BIT;
            VkAttachmentLoadOp mLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            VkAttachmentStoreOp mStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            VkAttachmentLoadOp mStencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            VkAttachmentStoreOp mStencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            VkImageLayout mInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkImageLayout mFinalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        };
        struct SubPass
        {
            byte mColorAttachments[8] = {};
            byte mColorAttachmentCount = 0;
        };

        void Load(const VulkanRenderPassInfo& renderPassInfo);
        size_t Hash(bool compatible) const;
        bool Equal(const RenderPassCacheEntry& rhs, bool compatible) const;

        template<bool CompatiblePolicy>
        struct HashPolicy : public Plasma::HashPolicy<RenderPassCacheEntry>
        {
            inline size_t operator()(const RenderPassCacheEntry& entry)
            {
                return entry.Hash(CompatiblePolicy);
            }
            inline bool Equal(const RenderPassCacheEntry& left, const RenderPassCacheEntry& right) const
            {
                return left.Equal(right, CompatiblePolicy);
            }
        };

        uint32_t mBaseLayer = 0;
        uint32_t mNumLayers = 1;
        byte mColorAttachmentCount = 0;
        AttachmentDescription mColorDescriptions[8] = {};
        AttachmentDescription mDepthDescription = {};
        Array<SubPass> mSubPasses;
    };

    //-------------------------------------------------------------------RenderPassCache
    class RenderPassCache
    {
    public:
        using RenderPassCookie = TypedCookie<VulkanRenderPass>;
        RenderPassCache(VkDevice device);
        ~RenderPassCache();

        VulkanRenderPass* FindOrCreate(const VulkanRenderPassInfo& renderPassInfo);
        VulkanRenderPass* FindOrCreateCompatible(const VulkanRenderPassInfo& renderPassInfo);

        VulkanRenderPass* Find(const RenderPassCookie& cookie);
        VulkanRenderPass* FindCompatible(const RenderPassCookie& cookie);

        void Free();

    private:
        VulkanRenderPass* Create(const VulkanRenderPassInfo& renderPassInfo, const RenderPassCacheEntry& cacheEntry);

        using CompatibleHashingPolicy = RenderPassCacheEntry::HashPolicy<true>;
        using ExactHashingPolicy = RenderPassCacheEntry::HashPolicy<false>;
        HashMap<RenderPassCacheEntry, VulkanRenderPass*, CompatibleHashingPolicy> mCompatibleRenderPasses;
        HashMap<RenderPassCacheEntry, VulkanRenderPass*, ExactHashingPolicy> mRenderPasses;
        HashMap<RenderPassCookie, VulkanRenderPass*> mRenderPassCookieMap;
        HashMap<RenderPassCookie, RenderPassCookie> mCompatibleRenderPassCookieMap;
        VkDevice mDevice;
    };

}
#pragma once

namespace Plasma
{
    class VulkanImageView;

    //-------------------------------------------------------------------VulkanRenderPassInfo
    class VulkanRenderPassInfo
    {
    public:
        VulkanImageView* mColorAttachments[8] = { nullptr };
        VulkanImageView* mDepthAttachment = nullptr;
        byte mColorAttachmentCount = 0;
        uint32_t mBaseLayer = 0;
        uint32_t mNumLayers = 1;

        struct AttachmentDescription
        {
            VkAttachmentLoadOp mLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            VkAttachmentStoreOp mStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            VkAttachmentLoadOp mStencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            VkAttachmentStoreOp mStencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            VkImageLayout mInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkImageLayout mFinalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        };

        AttachmentDescription mColorDescriptions[8] = {};
        AttachmentDescription mDepthDescription = {};

        struct SubPass
        {
            byte mColorAttachments[8] = {};
            byte mColorAttachmentCount = 0;
        };
        Array<SubPass> mSubPasses;
    };

    //-------------------------------------------------------------------VulkanRenderPass
    class VulkanRenderPass : public TypedCookie<VulkanRenderPass>
    {
    public:
        VulkanRenderPass(VkDevice device, const VulkanRenderPassInfo& info);
        ~VulkanRenderPass();

        void Free();

        VkRenderPass GetVulkanRenderPass() const;

    private:
        VkRenderPass mRenderPass = VK_NULL_HANDLE;
        VkDevice mDevice = VK_NULL_HANDLE;
    };


}
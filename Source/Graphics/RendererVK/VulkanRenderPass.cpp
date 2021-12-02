#include "Precompiled.hpp"

namespace Plasma
{
	//-------------------------------------------------------------------VulkanRenderPass
	VulkanRenderPass::VulkanRenderPass(VkDevice device, const VulkanRenderPassInfo& info)
	{
		mDevice = device;

		uint32_t totalAttachmentCount = info.mColorAttachmentCount + 1;
		VkAttachmentDescription attachmentDescriptions[9] = {};
		for (size_t i = 0; i < info.mColorAttachmentCount; ++i)
		{
			VulkanRenderPassInfo::AttachmentDescription desc = info.mColorDescriptions[i];
			VkAttachmentDescription& colorAttachment = attachmentDescriptions[i];
			colorAttachment.format = info.mColorAttachments[i]->GetImageFormat();
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = desc.mLoadOp;
			colorAttachment.storeOp = desc.mStoreOp;
			colorAttachment.stencilLoadOp = desc.mStencilLoadOp;
			colorAttachment.stencilStoreOp = desc.mStencilStoreOp;
			colorAttachment.initialLayout = desc.mInitialLayout;
			colorAttachment.finalLayout = desc.mFinalLayout;
		}

		VkAttachmentDescription& depthAttachment = attachmentDescriptions[info.mColorAttachmentCount];
		depthAttachment.format = info.mDepthAttachment->GetImageFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = info.mDepthDescription.mLoadOp;
		depthAttachment.storeOp = info.mDepthDescription.mStoreOp;
		depthAttachment.stencilLoadOp = info.mDepthDescription.mStencilLoadOp;
		depthAttachment.stencilStoreOp = info.mDepthDescription.mStencilStoreOp;
		depthAttachment.initialLayout = info.mDepthDescription.mInitialLayout;
		depthAttachment.finalLayout = info.mDepthDescription.mFinalLayout;


		Array<VkAttachmentReference> attachments;
		attachments.Reserve(1024);

		VkSubpassDescription subPasses[8] = {};
		for (size_t subPassIndex = 0; subPassIndex < info.mSubPasses.Size(); ++subPassIndex)
		{
			auto&& subPassInfo = info.mSubPasses[subPassIndex];
			VkSubpassDescription& subPassDesc = subPasses[subPassIndex];

			size_t colorAttachmentsIndex = attachments.Size();
			for (size_t colorIndex = 0; colorIndex < subPassInfo.mColorAttachmentCount; ++colorIndex)
			{
				VkAttachmentReference& attachment = attachments.PushBack();
				attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachment.attachment = subPassInfo.mColorAttachments[colorIndex];
			}
			size_t depthAttachmentIndex = attachments.Size();
			VkAttachmentReference& depthAttachment = attachments.PushBack();
			depthAttachment.attachment = subPassInfo.mColorAttachmentCount;
			depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			subPassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subPassDesc.colorAttachmentCount = subPassInfo.mColorAttachmentCount;
			subPassDesc.pColorAttachments = &attachments[colorAttachmentsIndex];
			subPassDesc.pDepthStencilAttachment = &attachments[depthAttachmentIndex];
		}

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dependency.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = totalAttachmentCount;
		renderPassInfo.pAttachments = attachmentDescriptions;
		renderPassInfo.subpassCount = static_cast<uint32_t>(info.mSubPasses.Size());
		renderPassInfo.pSubpasses = subPasses;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
			AlwaysError("failed to create render pass!");
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		Free();
	}

	void VulkanRenderPass::Free()
	{
		vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
		mRenderPass = VK_NULL_HANDLE;
	}

	VkRenderPass VulkanRenderPass::GetVulkanRenderPass() const
	{
		return mRenderPass;
	}

}
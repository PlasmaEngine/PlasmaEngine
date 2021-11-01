#pragma once

namespace Plasma
{
    struct SyncObjects
    {
        Array<VkSemaphore> mImageAvailableSemaphores;
        Array<VkSemaphore> mRenderFinishedSemaphores;
        Array<VkFence> mInFlightFences;
        Array<VkFence> mImagesInFlight;
    };
}
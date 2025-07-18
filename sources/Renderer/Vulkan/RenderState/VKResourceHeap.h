/*
 * VKResourceHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_RESOURCE_HEAP_H
#define LLGL_VK_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>
#include "VKPipelineBarrier.h"
#include "VKPipelineLayout.h"
#include "../Vulkan.h"
#include "../VKPtr.h"
#include <vector>


namespace LLGL
{


class Buffer;
class VKBuffer;
class VKTexture;
class VKDescriptorSetWriter;
class VKPipelineBarrier;
struct ResourceHeapDescriptor;
struct ResourceViewDescriptor;
struct TextureViewDescriptor;

class VKResourceHeap final : public ResourceHeap
{

    public:

        std::uint32_t GetNumDescriptorSets() const override;

    public:

        VKResourceHeap(
            VkDevice                                    device,
            const ResourceHeapDescriptor&               desc,
            const ArrayView<ResourceViewDescriptor>&    initialResourceViews = {}
        );

        std::uint32_t WriteResourceViews(
            VkDevice                                    device,
            std::uint32_t                               firstDescriptor,
            const ArrayView<ResourceViewDescriptor>&    resourceViews
        );

        // Sets all the barrier slots in the specified pipeline barrier.
        void SetBarrierSlots(VKPipelineBarrier& barrier, std::uint32_t descriptorSet);

        // Returns the native Vulkan descritpor pool.
        inline VkDescriptorPool GetVkDescriptorPool() const
        {
            return descriptorPool_.Get();
        }

        // Returns the list of native Vulkan descriptor sets.
        inline const std::vector<VkDescriptorSet>& GetVkDescriptorSets() const
        {
            return descriptorSets_;
        }

    private:

        static constexpr std::uint32_t invalidViewIndex = 0xFFFF;

        // Note that this struct will assign its parent member 'VKLayoutBinding::barrierSlot' a new value that is an index into 'barrierSlots_'.
        struct VKLayoutHeapBinding : VKLayoutBinding
        {
            std::uint32_t imageViewIndex  : 16; // Index (per descriptor set) to the intermediate VkImageView or 0xFFFF if unused.
            std::uint32_t bufferViewIndex : 16; // Index (per descriptor set) to the intermediate VkBufferView or 0xFFFF if unused.
        };

        union VKBarrierResource
        {
            inline VKBarrierResource() :
                buffer { VK_NULL_HANDLE }
            {
            }

            inline VKBarrierResource(VkBuffer buffer) :
                buffer { buffer }
            {
            }

            #if VK_USE_64_BIT_PTR_DEFINES
            inline VKBarrierResource(VkImage image) :
                image { image }
            {
            }
            #endif // /VK_USE_64_BIT_PTR_DEFINES

            VkBuffer    buffer;
            VkImage     image;
        };

    private:

        void ConvertAllLayoutBindings(const ArrayView<VKLayoutBinding>& layoutBindings);
        void ConvertLayoutBinding(VKLayoutHeapBinding& dst, const VKLayoutBinding& src);

        void CreateDescriptorPool(VkDevice device, std::uint32_t numDescriptorSets);

        void CreateDescriptorSets(
            VkDevice                device,
            std::uint32_t           numDescriptorSets,
            VkDescriptorSetLayout   globalSetLayout
        );

        void FillWriteDescriptorWithSampler(
            const ResourceViewDescriptor&   desc,
            std::uint32_t                   descriptorSet,
            const VKLayoutHeapBinding&      binding,
            VKDescriptorSetWriter&          setWriter
        );

        void FillWriteDescriptorWithImageView(
            VkDevice                        device,
            const ResourceViewDescriptor&   desc,
            std::uint32_t                   descriptorSet,
            const VKLayoutHeapBinding&      binding,
            VKDescriptorSetWriter&          setWriter
        );

        void FillWriteDescriptorWithBufferRange(
            VkDevice                        device,
            const ResourceViewDescriptor&   desc,
            std::uint32_t                   descriptorSet,
            const VKLayoutHeapBinding&      binding,
            VKDescriptorSetWriter&          setWriter
        );

        // Returns the image view for the specified texture or creates one if the texture-view is enabled.
        VkImageView GetOrCreateImageView(
            VkDevice                        device,
            VKTexture&                      textureVK,
            const ResourceViewDescriptor&   desc,
            std::size_t                     imageViewIndex
        );

        // Returns the buffer view for the specified buffer or creates one if the buffer-view is enabled.
        VkBufferView GetOrCreateBufferView(
            VkDevice                        device,
            VKBuffer&                       bufferVK,
            const ResourceViewDescriptor&   desc,
            std::size_t                     bufferViewIndex
        );

        // Allocates the buffer/image barrier slots for all descriptor sets.
        void AllocateBarrierSlots(std::uint32_t numDescriptorSets);

    private:

        VKPtr<VkDescriptorPool>             descriptorPool_;
        std::vector<VkDescriptorSet>        descriptorSets_;
        SmallVector<VKLayoutHeapBinding>    bindings_;

        std::vector<VKPtr<VkImageView>>     imageViews_;
        std::vector<VKPtr<VkBufferView>>    bufferViews_;
        std::uint32_t                       numImageViewsPerSet_    = 0;
        std::uint32_t                       numBufferViewsPerSet_   = 0;

        std::uint32_t                       numBufferBarriers_  : 16;
        std::uint32_t                       numImageBarriers_   : 16;
        SmallVector<std::uint32_t, 2>       barrierSlots_;
        std::vector<VKBarrierResource>      barrierResources_;

};


} // /namespace LLGL


#endif



// ================================================================================

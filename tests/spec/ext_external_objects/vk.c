/*
 * Copyright © 2020 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author:
 *    Eleni Maria Stea <estea@igalia.com>
 *    Juan A. Suarez Romero <jasuarez@igalia.com>
 */

#include "vk.h"

/* Vulkan create functions */
static VkInstance
create_instance(void)
{
	VkApplicationInfo app_info;
	VkInstanceCreateInfo inst_info;
	VkInstance inst;

	memset(&app_info, 0, sizeof app_info);
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "vktest";
	app_info.apiVersion = VK_MAKE_VERSION(1, 1, 0);

	memset(&inst_info, 0, sizeof inst_info);
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pApplicationInfo = &app_info;

	if (vkCreateInstance(&inst_info, 0, &inst) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return inst;
}

static VkPhysicalDevice
select_physical_device(VkInstance inst)
{
	VkResult res = VK_SUCCESS;
	uint32_t dev_count = 0;
	VkPhysicalDevice *pdevices;
	VkPhysicalDevice pdevice0;

	if ((res =
	     vkEnumeratePhysicalDevices(inst, &dev_count, 0)) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	pdevices = malloc(dev_count * sizeof(VkPhysicalDevice));
	if (vkEnumeratePhysicalDevices(inst, &dev_count, pdevices) !=
	    VK_SUCCESS)
		return VK_NULL_HANDLE;

	pdevice0 = pdevices[0];
	free(pdevices);

	return pdevice0;
}

static VkDevice
create_device(VkPhysicalDevice pdev)
{
	const char *deviceExtensions[] = { "VK_KHR_external_memory_fd" };
	VkDeviceQueueCreateInfo dev_queue_info;
	VkDeviceCreateInfo dev_info;
	VkDevice dev;

	memset(&dev_queue_info, 0, sizeof dev_queue_info);
	dev_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	dev_queue_info.queueFamilyIndex = 0;
	dev_queue_info.queueCount = 1;
	dev_queue_info.pQueuePriorities = (float[]) { 1.0f };

	memset(&dev_info, 0, sizeof dev_info);
	dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dev_info.queueCreateInfoCount = 1;
	dev_info.pQueueCreateInfos = &dev_queue_info;
	dev_info.enabledExtensionCount = 1;
	dev_info.ppEnabledExtensionNames = deviceExtensions;

	if (vkCreateDevice(pdev, &dev_info, 0, &dev) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return dev;
}

static VkPipelineCache
create_pipeline_cache(VkDevice dev)
{
	VkPipelineCacheCreateInfo pcache_info;
	VkPipelineCache pcache;

	memset(&pcache_info, 0, sizeof pcache_info);
	pcache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	if (vkCreatePipelineCache(dev, &pcache_info, 0, &pcache) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return pcache;
}

static VkCommandPool
create_cmd_pool(VkDevice dev)
{
	VkCommandPoolCreateInfo cmd_pool_info;
	VkCommandPool cmd_pool;

	memset(&cmd_pool_info, 0, sizeof cmd_pool_info);
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.queueFamilyIndex = 0;
	cmd_pool_info.flags = 0;

	if (vkCreateCommandPool(dev, &cmd_pool_info, 0, &cmd_pool) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return cmd_pool;
}

static VkCommandBuffer
create_cmd_buf(VkDevice dev, VkCommandPool cmd_pool)
{
	VkCommandBuffer cmd_buf;
	VkCommandBufferAllocateInfo alloc_info;

	memset(&alloc_info, 0, sizeof alloc_info);
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = 1;
	alloc_info.commandPool = cmd_pool;

	if (vkAllocateCommandBuffers(dev, &alloc_info, &cmd_buf) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return cmd_buf;
}

static inline VkImageType
get_image_type(uint32_t h, uint32_t d)
{
	if (h == 1)
		return VK_IMAGE_TYPE_1D;

	if (d > 1)
		return VK_IMAGE_TYPE_3D;

	return VK_IMAGE_TYPE_2D;
}

static uint32_t
get_memory_type_idx(VkPhysicalDevice pdev,
		    const VkMemoryRequirements *mem_reqs,
		    VkMemoryPropertyFlagBits prop_flags)
{
	VkPhysicalDeviceMemoryProperties pdev_mem_props;
	uint32_t i;

	vkGetPhysicalDeviceMemoryProperties(pdev, &pdev_mem_props);

	for (i = 0; i < pdev_mem_props.memoryTypeCount; i++) {
		const VkMemoryType *type = &pdev_mem_props.memoryTypes[i];

		if ((mem_reqs->memoryTypeBits & (1 << i)) &&
		    (type->propertyFlags & prop_flags) == prop_flags) {
			return i;
			break;
		}
	}
	return UINT32_MAX;
}

static VkDeviceMemory
alloc_memory(struct vk_ctx *ctx,
	     const VkMemoryRequirements *mem_reqs,
	     VkMemoryPropertyFlagBits prop_flags)
{
	VkExportMemoryAllocateInfo exp_mem_info;
	VkMemoryAllocateInfo mem_alloc_info;
	VkDeviceMemory mem;

	memset(&exp_mem_info, 0, sizeof exp_mem_info);
	exp_mem_info.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
	exp_mem_info.handleTypes =
		VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

	memset(&mem_alloc_info, 0, sizeof mem_alloc_info);
	mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc_info.pNext = &exp_mem_info;
	mem_alloc_info.allocationSize = mem_reqs->size;
	mem_alloc_info.memoryTypeIndex =
		get_memory_type_idx(ctx->pdev, mem_reqs, prop_flags);

	if (mem_alloc_info.memoryTypeIndex == UINT32_MAX) {
		fprintf(stderr, "No suitable memory type index found.\n");
		return VK_NULL_HANDLE;
	}

	if (vkAllocateMemory(ctx->dev, &mem_alloc_info, 0, &mem) !=
	    VK_SUCCESS)
		return VK_NULL_HANDLE;

	return mem;
}

static bool
alloc_image_memory(struct vk_ctx *ctx, struct vk_image_obj *img_obj)
{
	VkImageMemoryRequirementsInfo2 req_info2;
	VkMemoryRequirements2 mem_reqs2;

	/* VkImageMemoryRequirementsInfo2 */
	memset(&req_info2, 0, sizeof req_info2);
	req_info2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
	req_info2.image = img_obj->img;

	/* VkMemoryRequirements2 */
	memset(&mem_reqs2, 0, sizeof mem_reqs2);
	mem_reqs2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;

	vkGetImageMemoryRequirements2(ctx->dev, &req_info2, &mem_reqs2);
	img_obj->mem = alloc_memory(ctx,
				    &mem_reqs2.memoryRequirements,
				    mem_reqs2.memoryRequirements.memoryTypeBits &
				    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	img_obj->mem_sz = mem_reqs2.memoryRequirements.size;
	if (img_obj->mem == VK_NULL_HANDLE) {
		fprintf(stderr, "Failed to allocate image memory.\n");
		return false;
	}
	return true;
}

static bool
are_props_supported(struct vk_ctx *ctx, struct vk_image_props *props)
{
	VkPhysicalDeviceExternalImageFormatInfo ext_img_fmt_info;
	VkPhysicalDeviceImageFormatInfo2 img_fmt_info;
	VkExternalImageFormatProperties ext_img_fmt_props;
	VkImageFormatProperties2 img_fmt_props;

	VkExternalMemoryFeatureFlagBits feature_flags =
		VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT;
	VkExternalMemoryHandleTypeFlagBits handle_type =
		VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

	memset(&ext_img_fmt_info, 0, sizeof ext_img_fmt_info);
	ext_img_fmt_info.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
	ext_img_fmt_info.handleType = handle_type;

	memset(&img_fmt_info, 0, sizeof img_fmt_info);
	img_fmt_info.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
	img_fmt_info.pNext = &ext_img_fmt_info;
	img_fmt_info.format = props->format;
	img_fmt_info.type = get_image_type(props->h, props->depth);
	img_fmt_info.tiling = props->tiling;
	img_fmt_info.usage = props->usage;

	memset(&ext_img_fmt_props, 0, sizeof ext_img_fmt_props);
	ext_img_fmt_props.sType =
		VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES;

	memset(&img_fmt_props, 0, sizeof img_fmt_props);
	img_fmt_props.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
	img_fmt_props.pNext = &ext_img_fmt_props;

	if (vkGetPhysicalDeviceImageFormatProperties2
	    (ctx->pdev, &img_fmt_info, &img_fmt_props) != VK_SUCCESS) {
		fprintf(stderr,
			"Unsupported Vulkan format properties.\n");
		return false;
	}

	if (!(ext_img_fmt_props.externalMemoryProperties.externalMemoryFeatures & feature_flags)) {
		fprintf(stderr, "Unsupported Vulkan external memory features.\n");
		return false;
	}

	return true;
}

/* Vulkan test visible functions */

bool
vk_init_ctx(struct vk_ctx *ctx)
{
	if ((ctx->inst = create_instance()) == VK_NULL_HANDLE) {
		fprintf(stderr, "Failed to create Vulkan instance.\n");
		goto fail;
	}

	if ((ctx->pdev = select_physical_device(ctx->inst)) == VK_NULL_HANDLE) {
		fprintf(stderr, "Failed to find suitable physical device.\n");
		goto fail;
	}

	if ((ctx->dev = create_device(ctx->pdev)) == VK_NULL_HANDLE) {
		fprintf(stderr, "Failed to create Vulkan device.\n");
		goto fail;
	}
	return true;

fail:
	vk_cleanup_ctx(ctx);
	return false;
}

bool
vk_init_ctx_for_rendering(struct vk_ctx *ctx)
{
	if (!vk_init_ctx(ctx)) {
		fprintf(stderr, "Failed to initialize Vulkan.\n");
		goto fail;
	}

	if ((ctx->cache = create_pipeline_cache(ctx->dev)) == VK_NULL_HANDLE) {
		fprintf(stderr, "Failed to create pipeline cache.\n");
		goto fail;
	}

	if ((ctx->cmd_pool = create_cmd_pool(ctx->dev)) == VK_NULL_HANDLE) {
		fprintf(stderr, "Failed to create command pool.\n");
		goto fail;
	}

	if ((ctx->cmd_buf = create_cmd_buf(ctx->dev, ctx->cmd_pool)) ==
			VK_NULL_HANDLE) {
		fprintf(stderr, "Failed to create command buffer.\n");
		goto fail;
	}

	vkGetDeviceQueue(ctx->dev, 0, 0, &ctx->queue);
	if (!ctx->queue) {
		fprintf(stderr, "Failed to get command queue.\n");
		goto fail;
	}

	return true;

fail:
	vk_cleanup_ctx(ctx);
	return false;
}

void
vk_cleanup_ctx(struct vk_ctx *ctx)
{
	if (ctx->cmd_buf != VK_NULL_HANDLE)
		vkFreeCommandBuffers(ctx->dev, ctx->cmd_pool, 1, &ctx->cmd_buf);

	if (ctx->cmd_pool != VK_NULL_HANDLE)
		vkDestroyCommandPool(ctx->dev, ctx->cmd_pool, 0);

	if (ctx->cache != VK_NULL_HANDLE)
		vkDestroyPipelineCache(ctx->dev, ctx->cache, 0);

	if (ctx->dev != VK_NULL_HANDLE)
		vkDestroyDevice(ctx->dev, 0);

	if (ctx->inst != VK_NULL_HANDLE)
		vkDestroyInstance(ctx->inst, 0);
}

bool
vk_create_ext_image(struct vk_ctx *ctx,
		    struct vk_image_props *props, struct vk_image_obj *img)
{
	VkExternalMemoryImageCreateInfo ext_img_info;
	VkImageCreateInfo img_info;

	memset(&ext_img_info, 0, sizeof ext_img_info);
	ext_img_info.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
	ext_img_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

	memset(&img_info, 0, sizeof img_info);
	img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	img_info.pNext = &ext_img_info;
	img_info.imageType = get_image_type(props->h, props->depth);
	img_info.format = props->format;
	img_info.extent.width = props->w;
	img_info.extent.height = props->h;
	img_info.extent.depth = props->depth;
	img_info.mipLevels = props->num_levels ? props->num_levels : 1;
	img_info.arrayLayers = 1;
	img_info.samples = props->num_samples ? (VkSampleCountFlagBits)props->num_samples :
						VK_SAMPLE_COUNT_1_BIT;
	img_info.tiling = props->tiling;
	img_info.usage = props->usage;
	img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (vkCreateImage(ctx->dev, &img_info, 0, &img->img) != VK_SUCCESS)
		goto fail;

	if(!alloc_image_memory(ctx, img))
		goto fail;

	return true;

fail:
	fprintf(stderr, "Failed to create external image.\n");
	vk_destroy_ext_image(ctx, img);
	img->img = VK_NULL_HANDLE;
	img->mem = VK_NULL_HANDLE;
	return false;
}

void
vk_destroy_ext_image(struct vk_ctx *ctx, struct vk_image_obj *img_obj)
{
	if (img_obj->img != VK_NULL_HANDLE)
		vkDestroyImage(ctx->dev, img_obj->img, 0);

	if (img_obj->mem != VK_NULL_HANDLE)
		vkFreeMemory(ctx->dev, img_obj->mem, 0);
}

bool
vk_fill_ext_image_props(struct vk_ctx *ctx,
			uint32_t w,
			uint32_t h,
			uint32_t d,
			uint32_t num_samples,
			uint32_t num_levels,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlagBits usage,
			struct vk_image_props *props)
{
	props->w = w;
	props->h = h;
	props->depth = d;

	props->num_samples = num_samples;
	props->num_levels = num_levels;

	props->format = format;
	props->usage = usage;
	props->tiling = tiling;

	if (!are_props_supported(ctx, props))
		return false;

	return true;
}

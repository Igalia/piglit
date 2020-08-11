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
 *    Topi Pohjolainen <topi.pohjolainen@intel.com>
 */

#include "vk.h"

/* static variables */
static VkViewport viewport;
static VkRect2D scissor;

/* static functions */
static VkSampleCountFlagBits
get_num_samples(uint32_t num_samples);

/* Vulkan create functions */

static void
enable_validation_layers(VkInstanceCreateInfo *info)
{
	int i;
	uint32_t num_layers;
	VkLayerProperties *layers;
	static const char *layer_names[] = {
		"VK_LAYER_KHRONOS_validation",
	};

	vkEnumerateInstanceLayerProperties(&num_layers, 0);
	layers = alloca(num_layers * sizeof *layers);
	vkEnumerateInstanceLayerProperties(&num_layers, layers);

	if (num_layers) {
		printf("Available validation layers:\n");
		for(i = 0; i < (int)num_layers; i++) {
			printf(" %s\n", layers[i].layerName);
		}

		info->ppEnabledLayerNames = layer_names;
		info->enabledLayerCount = sizeof layer_names / sizeof *layer_names;
	} else {
		fprintf(stderr, "Vulkan validation layers not found.\n");
	}
}

static VkInstance
create_instance(bool enable_layers)
{
	VkApplicationInfo app_info;
	VkInstanceCreateInfo inst_info;
	VkInstance inst;

	memset(&app_info, 0, sizeof app_info);
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "vktest";
	app_info.apiVersion = VK_API_VERSION_1_1;

	memset(&inst_info, 0, sizeof inst_info);
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pApplicationInfo = &app_info;

	if (enable_layers)
		enable_validation_layers(&inst_info);

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
create_device(struct vk_ctx *ctx, VkPhysicalDevice pdev)
{
	const char *deviceExtensions[] = { "VK_KHR_external_memory_fd",
					   "VK_KHR_external_semaphore_fd" };
	VkDeviceQueueCreateInfo dev_queue_info;
	VkDeviceCreateInfo dev_info;
	VkDevice dev;
	uint32_t prop_count;
	VkQueueFamilyProperties *fam_props;
	uint32_t i;
	float qprio = 0;

	ctx->qfam_idx = -1;
	vkGetPhysicalDeviceQueueFamilyProperties(pdev, &prop_count, 0);
	if (prop_count < 0) {
		fprintf(stderr, "Invalid queue family properties.\n");
		return VK_NULL_HANDLE;
	}

	fam_props = malloc(prop_count * sizeof *fam_props);
	vkGetPhysicalDeviceQueueFamilyProperties(pdev, &prop_count, fam_props);

	for (i = 0; i < prop_count; i++) {
		if (fam_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			ctx->qfam_idx = i;
			break;
		}
	}
	free(fam_props);

	memset(&dev_queue_info, 0, sizeof dev_queue_info);
	dev_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	dev_queue_info.queueFamilyIndex = ctx->qfam_idx;
	dev_queue_info.queueCount = 1;
	dev_queue_info.pQueuePriorities = &qprio;

	memset(&dev_info, 0, sizeof dev_info);
	dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dev_info.queueCreateInfoCount = 1;
	dev_info.pQueueCreateInfos = &dev_queue_info;
	dev_info.enabledExtensionCount = ARRAY_SIZE(deviceExtensions);
	dev_info.ppEnabledExtensionNames = deviceExtensions;

	if (vkCreateDevice(pdev, &dev_info, 0, &dev) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return dev;
}

static void
fill_uuid(VkPhysicalDevice pdev, uint8_t *deviceUUID, uint8_t *driverUUID)
{
	VkPhysicalDeviceIDProperties devProp;
	VkPhysicalDeviceProperties2 prop2;

	memset(&devProp, 0, sizeof devProp);
	devProp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;

	memset(&prop2, 0, sizeof prop2);
	prop2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	prop2.pNext = &devProp;

	vkGetPhysicalDeviceProperties2(pdev, &prop2);
	memcpy(deviceUUID, devProp.deviceUUID, VK_UUID_SIZE);
	memcpy(driverUUID, devProp.driverUUID, VK_UUID_SIZE);
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
create_cmd_pool(struct vk_ctx *ctx)
{
	VkCommandPoolCreateInfo cmd_pool_info;
	VkCommandPool cmd_pool;
	VkDevice dev = ctx->dev;

	memset(&cmd_pool_info, 0, sizeof cmd_pool_info);
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.queueFamilyIndex = ctx->qfam_idx;
	cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(dev, &cmd_pool_info, 0, &cmd_pool) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return cmd_pool;
}

static VkRenderPass
create_renderpass(struct vk_ctx *ctx,
		  struct vk_image_props *color_img_props,
		  struct vk_image_props *depth_img_props)
{
	uint32_t num_attachments = 2;
	VkAttachmentDescription att_dsc[2];
	VkAttachmentReference att_rfc[2];
	VkSubpassDescription subpass_dsc[1];
	VkRenderPassCreateInfo rpass_info;

	/* VkAttachmentDescription */
	memset(att_dsc, 0, num_attachments * sizeof att_dsc[0]);

	att_dsc[0].samples = get_num_samples(color_img_props->num_samples);
	att_dsc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	att_dsc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	att_dsc[0].initialLayout = color_img_props->in_layout;
	att_dsc[0].finalLayout = color_img_props->end_layout;
	att_dsc[0].format = color_img_props->format;

	att_dsc[1].samples = get_num_samples(depth_img_props->num_samples);
	att_dsc[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	att_dsc[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	att_dsc[1].initialLayout = depth_img_props->in_layout;
	att_dsc[1].finalLayout = depth_img_props->end_layout;
	att_dsc[1].format = depth_img_props->format;

	/* VkAttachmentReference */
	memset(att_rfc, 0, num_attachments * sizeof att_rfc[0]);

	att_rfc[0].layout = color_img_props->tiling == VK_IMAGE_TILING_OPTIMAL ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
	att_rfc[0].attachment = 0;

	att_rfc[1].layout = depth_img_props->tiling == VK_IMAGE_TILING_OPTIMAL ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
	att_rfc[1].attachment = 1;

	/* VkSubpassDescription */
	memset(&subpass_dsc, 0, sizeof subpass_dsc);
	subpass_dsc[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_dsc[0].colorAttachmentCount = 1;
	subpass_dsc[0].pColorAttachments = &att_rfc[0];
	subpass_dsc[0].pDepthStencilAttachment = &att_rfc[1];

	/* VkRenderPassCreateInfo */
	memset(&rpass_info, 0, sizeof rpass_info);
	rpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpass_info.attachmentCount = num_attachments;
	rpass_info.pAttachments = att_dsc;
	rpass_info.subpassCount = 1;
	rpass_info.pSubpasses = subpass_dsc;

	VkRenderPass rpass;
	if (vkCreateRenderPass(ctx->dev, &rpass_info, 0, &rpass) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create renderpass.\n");
		rpass = VK_NULL_HANDLE;
	}

	return rpass;
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

static VkImageViewType
get_image_view_type(struct vk_image_props *props)
{
	VkImageType type = get_image_type(props->h, props->depth);
	switch(type) {
		case VK_IMAGE_TYPE_1D:
			return props->num_layers > 1 ?
				VK_IMAGE_VIEW_TYPE_1D_ARRAY :
				VK_IMAGE_VIEW_TYPE_1D;
		case VK_IMAGE_TYPE_2D:
			if (props->num_layers == 1)
				return VK_IMAGE_VIEW_TYPE_2D;
			if (props->num_layers == 6)
				return VK_IMAGE_VIEW_TYPE_CUBE;
			if (props->num_layers % 6 == 0)
				return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
			if (props->num_layers > 1)
				return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		case VK_IMAGE_TYPE_3D:
			if (props->num_layers == 1)
				return VK_IMAGE_VIEW_TYPE_3D;
			if ((props->num_layers == 1) &&
			    (props->num_levels == 1))
				return VK_IMAGE_VIEW_TYPE_2D;
			if ((props->num_levels == 1) &&
			    (props->num_layers > 1))
				return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		default:
			return VK_IMAGE_VIEW_TYPE_2D;
	}
}

static VkImageAspectFlagBits
get_aspect_from_depth_format(VkFormat depth_format)
{
	switch (depth_format) {
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_X8_D24_UNORM_PACK32:
	case VK_FORMAT_D32_SFLOAT:
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	case VK_FORMAT_S8_UINT:
		return VK_IMAGE_ASPECT_STENCIL_BIT;
	case VK_FORMAT_D16_UNORM_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	default:
		break;
	}
	return VK_NULL_HANDLE;
}

static VkPipelineStageFlags
get_pipeline_stage_flags(const VkImageLayout layout)
{
	switch (layout) {
	case VK_IMAGE_LAYOUT_UNDEFINED:
		return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	case VK_IMAGE_LAYOUT_GENERAL:
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		return VK_PIPELINE_STAGE_HOST_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
		       VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	default:
		return 0;
	}
	return 0;
}

static void
create_framebuffer(struct vk_ctx *ctx,
		   struct vk_image_att *color_att,
		   struct vk_image_att *depth_att,
		   struct vk_renderer *renderer)
{
	VkImageSubresourceRange sr;
	VkImageViewCreateInfo color_info;
	VkImageViewCreateInfo depth_info;
	VkFramebufferCreateInfo fb_info;
	VkImageView atts[2];
	VkImageViewType view_type = get_image_view_type(&color_att->props);

	if (!color_att->obj.img || !depth_att->obj.img) {
		fprintf(stderr, "Invalid framebuffer attachment image.\n");
		goto fail;
	}

	/* create image views */

	/* VKImageSubresourceRange */
	memset(&sr, 0, sizeof sr);
	sr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	/* If an application wants to use all mip levels
	 * or layers in an image after the baseMipLevel
	 * or baseArrayLayer, it can set levelCount and
	 * layerCount to the special values
	 * VK_REMAINING_MIP_LEVELS and
	 * VK_REMAINING_ARRAY_LAYERS without knowing the
	 * exact number of mip levels or layers.
	 */
	sr.baseMipLevel = 0;
	sr.levelCount = color_att->props.num_levels;
	sr.baseArrayLayer = 0;
	sr.layerCount = color_att->props.num_layers;

	/* color view */
	memset(&color_info, 0, sizeof color_info);
	color_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	color_info.image = color_att->obj.img;
	color_info.viewType = view_type;
	color_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	color_info.format = color_att->props.format;
	color_info.subresourceRange = sr;

	if (vkCreateImageView(ctx->dev, &color_info, 0, &atts[0]) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create color image view for framebuffer.\n");
		vk_destroy_ext_image(ctx, &color_att->obj);
		goto fail;
	}

	/* depth view */
	memset(&sr, 0, sizeof sr);
	sr.aspectMask = get_aspect_from_depth_format(depth_att->props.format);
	sr.baseMipLevel = 0;
	sr.levelCount = depth_att->props.num_levels ? depth_att->props.num_levels : 1;
	sr.baseArrayLayer = 0;
	sr.layerCount = depth_att->props.num_layers ? depth_att->props.num_layers : 1;

	memset(&depth_info, 0, sizeof depth_info);
	depth_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depth_info.image = depth_att->obj.img;
	depth_info.viewType = depth_att->props.num_layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
	depth_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depth_info.format = depth_att->props.format;
	depth_info.subresourceRange = sr;

	if (vkCreateImageView(ctx->dev, &depth_info, 0, &atts[1]) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create depth image view for framebuffer.\n");
		vk_destroy_ext_image(ctx, &depth_att->obj);
		goto fail;
	}

	memset(&fb_info, 0, sizeof fb_info);
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.renderPass = renderer->renderpass;
	fb_info.width = color_att->props.w;
	fb_info.height = color_att->props.h;
	fb_info.layers = color_att->props.num_layers ? color_att->props.num_layers : 1;
	fb_info.attachmentCount = 2;
	fb_info.pAttachments = atts;

	if (vkCreateFramebuffer(ctx->dev, &fb_info, 0, &renderer->fb) != VK_SUCCESS)
		goto fail;

	return;

fail:
	fprintf(stderr, "Failed to create framebuffer.\n");
	renderer->fb = VK_NULL_HANDLE;
}

static VkShaderModule
create_shader_module(struct vk_ctx *ctx,
		     const char *src,
		     unsigned int size)
{
	VkShaderModuleCreateInfo sm_info;
	VkShaderModule sm;

	/* VkShaderModuleCreateInfo */
	memset(&sm_info, 0, sizeof sm_info);
	sm_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	sm_info.codeSize = size;
	sm_info.pCode = (void*)src;

	if (vkCreateShaderModule(ctx->dev, &sm_info, 0, &sm) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create shader module.\n");
		sm = VK_NULL_HANDLE;
	}

	return sm;
}

static void
create_pipeline(struct vk_ctx *ctx,
		uint32_t width,
		uint32_t height,
		uint32_t num_samples,
		bool enable_depth,
		bool enable_stencil,
		struct vk_renderer *renderer)
{
	VkPipelineColorBlendAttachmentState cb_att_state[1];
	VkPipelineVertexInputStateCreateInfo vert_input_info;
	VkPipelineInputAssemblyStateCreateInfo asm_info;
	VkPipelineViewportStateCreateInfo viewport_info;
	VkPipelineRasterizationStateCreateInfo rs_info;
	VkPipelineMultisampleStateCreateInfo ms_info;
	VkPipelineDepthStencilStateCreateInfo ds_info;
	VkPipelineColorBlendStateCreateInfo cb_info;
	VkPipelineShaderStageCreateInfo sdr_stages[2];
	VkPipelineLayoutCreateInfo layout_info;
	VkGraphicsPipelineCreateInfo pipeline_info;
	VkFormat format;
	VkFormatProperties fmt_props;
	VkPushConstantRange pc_range[1];

	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	VkStencilOpState front;
	VkStencilOpState back;
	int i;
	VkPipelineLayout pipeline_layout;

	/* format of vertex attributes:
	 * we have 2D vectors so we need a RG format:
	 * R for x, G for y
	 * the stride (distance between 2 consecutive elements)
	 * must be 8 because we use 32 bit floats and
	 * 32bits = 8bytes */
	format = VK_FORMAT_R32G32_SFLOAT;
	vkGetPhysicalDeviceFormatProperties(ctx->pdev, format, &fmt_props);
	assert(fmt_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT);

	/* VkPipelineVertexInputStateCreateInfo */
	memset(&vert_input_info, 0, sizeof vert_input_info);
	vert_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	/* VkPipelineInputAssemblyStateCreateInfo */
	memset(&asm_info, 0, sizeof asm_info);
	asm_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	asm_info.topology = topology;
	asm_info.primitiveRestartEnable = false;

	/* VkViewport */
	viewport.x = viewport.y = 0;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;

	/* VkRect2D scissor */
	scissor.offset.x = scissor.offset.y = 0;
	scissor.extent.width = width;
	scissor.extent.height = height;

	/* VkPipelineViewportStateCreateInfo */
	memset(&viewport_info, 0, sizeof viewport_info);
	viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.viewportCount = 1;
	viewport_info.pViewports = &viewport;
	viewport_info.scissorCount = 1;
	viewport_info.pScissors = &scissor;

	/* VkPipelineRasterizationStateCreateInfo */
	memset(&rs_info, 0, sizeof rs_info);
	rs_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs_info.polygonMode = VK_POLYGON_MODE_FILL;
	rs_info.cullMode = VK_CULL_MODE_NONE;
	rs_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rs_info.lineWidth = 1.0;

	/* VkPipelineMultisampleStateCreateInfo */
	memset(&ms_info, 0, sizeof ms_info);
	ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms_info.rasterizationSamples = num_samples;

	/* VkStencilOpState */
	/* The default values for ES are taken by Topi Pohjolainen's code */
	/* defaults in OpenGL ES 3.1 */
	memset(&front, 0, sizeof front);
	front.compareMask = ~0;
	front.writeMask = ~0;
	front.reference = 0;

	memset(&back, 0, sizeof back);
	back.compareMask = ~0;
	back.writeMask = ~0;
	back.reference = 0;

	/* VkPipelineDepthStencilStateCreateInfo */
	memset(&ds_info, 0, sizeof ds_info);
	ds_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds_info.front = front;
	ds_info.back = back;
	/* defaults in OpenGL ES 3.1 */
	ds_info.minDepthBounds = 0;
	ds_info.maxDepthBounds = 1;
	/* z buffer, stencil buffer */
	if (enable_depth) {
		ds_info.depthTestEnable = VK_TRUE;
		ds_info.depthWriteEnable = VK_TRUE;
		ds_info.depthCompareOp = VK_COMPARE_OP_LESS;

		if (enable_stencil)
			fprintf(stderr, "Depth and stencil tests are enabled at the same time. Ignoring stencil.\n");

	} else if (enable_stencil) {
		ds_info.stencilTestEnable = VK_TRUE;
		ds_info.depthTestEnable = VK_FALSE;
		ds_info.depthWriteEnable = VK_FALSE;
		ds_info.depthCompareOp = VK_COMPARE_OP_LESS;
	}

	/* VkPipelineColorBlendAttachmentState */
	memset(&cb_att_state[0], 0, sizeof cb_att_state[0]);
	cb_att_state[0].colorWriteMask = (VK_COLOR_COMPONENT_R_BIT |
					  VK_COLOR_COMPONENT_G_BIT |
					  VK_COLOR_COMPONENT_B_BIT |
					  VK_COLOR_COMPONENT_A_BIT);

	/* VkPipelineColorBlendStateCreateInfo */
	memset(&cb_info, 0, sizeof cb_info);
	cb_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb_info.attachmentCount = 1;
	cb_info.pAttachments = cb_att_state;
	/* default in ES 3.1 */
	for (i = 0; i < 4; i++) {
		cb_info.blendConstants[i] = 0.0f;
	}

	/* VkPipelineShaderStageCreateInfo */
	memset(sdr_stages, 0, 2 * sizeof sdr_stages[0]);

	sdr_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	sdr_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	sdr_stages[0].module = renderer->vs;
	sdr_stages[0].pName = "main";

	sdr_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	sdr_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	sdr_stages[1].module = renderer->fs;
	sdr_stages[1].pName = "main";

	/* VkPushConstantRange */
	memset(pc_range, 0, sizeof pc_range[0]);
	pc_range[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pc_range[0].size = sizeof (struct vk_dims); /* w, h */

	/* VkPipelineLayoutCreateInfo */
	memset(&layout_info, 0, sizeof layout_info);
	layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_info.pushConstantRangeCount = 1;
	layout_info.pPushConstantRanges = pc_range;

	if (vkCreatePipelineLayout(ctx->dev, &layout_info, 0, &pipeline_layout) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create pipeline layout\n");
		renderer->pipeline = VK_NULL_HANDLE;
		return;
	}

	renderer->pipeline_layout = pipeline_layout;

	/* VkGraphicsPipelineCreateInfo */
	memset(&pipeline_info, 0, sizeof pipeline_info);
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.layout = pipeline_layout;
	pipeline_info.renderPass = renderer->renderpass;
	pipeline_info.pVertexInputState = &vert_input_info;
	pipeline_info.pInputAssemblyState = &asm_info;
	pipeline_info.pViewportState = &viewport_info;
	pipeline_info.pRasterizationState = &rs_info;
	pipeline_info.pMultisampleState = &ms_info;
	pipeline_info.pDepthStencilState = &ds_info;
	pipeline_info.pColorBlendState = &cb_info;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = sdr_stages;

	if (vkCreateGraphicsPipelines(ctx->dev, ctx->cache, 1,
				      &pipeline_info, 0, &renderer->pipeline) !=
			VK_SUCCESS) {
		fprintf(stderr, "Failed to create graphics pipeline.\n");
		renderer->pipeline = VK_NULL_HANDLE;
	}
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

static VkSampleCountFlagBits
get_num_samples(uint32_t num_samples)
{
	switch(num_samples) {
	case 64:
		return VK_SAMPLE_COUNT_64_BIT;
	case 32:
		return VK_SAMPLE_COUNT_32_BIT;
	case 16:
		return VK_SAMPLE_COUNT_16_BIT;
	case 8:
		return VK_SAMPLE_COUNT_8_BIT;
	case 4:
		return VK_SAMPLE_COUNT_4_BIT;
	case 2:
		return VK_SAMPLE_COUNT_2_BIT;
	case 1:
		break;
	default:
		fprintf(stderr, "Invalid number of samples in VkSampleCountFlagBits. Using one sample.\n");
		break;
	}
	return VK_SAMPLE_COUNT_1_BIT;
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
	img_obj->mobj.mem = alloc_memory(ctx,
					 &mem_reqs2.memoryRequirements,
					 mem_reqs2.memoryRequirements.memoryTypeBits &
					 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	img_obj->mobj.mem_sz = mem_reqs2.memoryRequirements.size;
	if (img_obj->mobj.mem == VK_NULL_HANDLE) {
		fprintf(stderr, "Failed to allocate image memory.\n");
		return false;
	}

	if (vkBindImageMemory(ctx->dev, img_obj->img, img_obj->mobj.mem, 0) != VK_SUCCESS) {
		fprintf(stderr, "Failed to bind image memory.\n");
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
	img_fmt_info.usage = props->usage ? props->usage : VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

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
	if ((ctx->inst = create_instance(false)) == VK_NULL_HANDLE) {
		fprintf(stderr, "Failed to create Vulkan instance.\n");
		goto fail;
	}

	if ((ctx->pdev = select_physical_device(ctx->inst)) == VK_NULL_HANDLE) {
		fprintf(stderr, "Failed to find suitable physical device.\n");
		goto fail;
	}

	if ((ctx->dev = create_device(ctx, ctx->pdev)) == VK_NULL_HANDLE) {
		fprintf(stderr, "Failed to create Vulkan device.\n");
		goto fail;
	}

	fill_uuid(ctx->pdev, ctx->deviceUUID, ctx->driverUUID);
	return true;

fail:
	vk_cleanup_ctx(ctx);
	return false;
}

static VkAccessFlagBits
get_access_mask(const VkImageLayout layout)
{
	switch (layout) {
	case VK_IMAGE_LAYOUT_UNDEFINED:
		return 0;
	case VK_IMAGE_LAYOUT_GENERAL:
		return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
		       VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT |
		       VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_HOST_WRITE_BIT |
		       VK_ACCESS_HOST_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
		       VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
		       VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		return VK_ACCESS_HOST_WRITE_BIT;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		return VK_ACCESS_TRANSFER_READ_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		return VK_ACCESS_TRANSFER_WRITE_BIT;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		return 0;
	default:
		return 0;
	};

	return 0;
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

	if ((ctx->cmd_pool = create_cmd_pool(ctx)) == VK_NULL_HANDLE) {
		fprintf(stderr, "Failed to create command pool.\n");
		goto fail;
	}

	if ((ctx->cmd_buf = create_cmd_buf(ctx->dev, ctx->cmd_pool)) ==
			VK_NULL_HANDLE) {
		fprintf(stderr, "Failed to create command buffer.\n");
		goto fail;
	}

	vkGetDeviceQueue(ctx->dev, ctx->qfam_idx, 0, &ctx->queue);
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
	img_info.arrayLayers = props->num_layers ? props->num_layers : VK_SAMPLE_COUNT_1_BIT;
	img_info.samples = get_num_samples(props->num_samples);
	img_info.tiling = props->tiling;
	img_info.usage = props->usage ? props->usage : VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
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
	img->mobj.mem = VK_NULL_HANDLE;
	return false;
}

bool
vk_create_ext_buffer(struct vk_ctx *ctx,
		     uint32_t sz,
		     VkBufferUsageFlagBits usage,
		     struct vk_buf *bo)
{
	VkExternalMemoryBufferCreateInfo ext_bo_info;

	memset(&ext_bo_info, 0, sizeof ext_bo_info);
	ext_bo_info.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
	ext_bo_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

	if (!vk_create_buffer(ctx, sz, usage, &ext_bo_info, bo)) {
		fprintf(stderr, "Failed to allocate external buffer.\n");
		return false;
	}

	return true;
}

void
vk_destroy_ext_image(struct vk_ctx *ctx, struct vk_image_obj *img_obj)
{
	if (img_obj->img != VK_NULL_HANDLE) {
		vkDestroyImage(ctx->dev, img_obj->img, 0);
		img_obj->img = VK_NULL_HANDLE;
	}

	if (img_obj->mobj.mem != VK_NULL_HANDLE) {
		vkFreeMemory(ctx->dev, img_obj->mobj.mem, 0);
		img_obj->mobj.mem = VK_NULL_HANDLE;
	}
}

void
vk_destroy_ext_bo(struct vk_ctx *ctx,
		  struct vk_buf *bo)
{
	if (bo->buf != VK_NULL_HANDLE) {
		vkDestroyBuffer(ctx->dev, bo->buf, 0);
		bo->buf = VK_NULL_HANDLE;
	}

	if (bo->mobj.mem != VK_NULL_HANDLE) {
		vkFreeMemory(ctx->dev, bo->mobj.mem, 0);
		bo->mobj.mem = VK_NULL_HANDLE;
	}
}

bool
vk_fill_ext_image_props(struct vk_ctx *ctx,
			uint32_t w,
			uint32_t h,
			uint32_t d,
			uint32_t num_samples,
			uint32_t num_levels,
			uint32_t num_layers,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlagBits usage,
			VkImageLayout in_layout,
			VkImageLayout end_layout,
			struct vk_image_props *props)
{
	props->w = w;
	props->h = h;
	props->depth = d;

	props->num_samples = num_samples;
	props->num_levels = num_levels;
	props->num_layers = num_layers;

	props->format = format;
	props->usage = usage;
	props->tiling = tiling;

	props->in_layout = in_layout;
	props->end_layout = end_layout;

	if (!are_props_supported(ctx, props))
		return false;

	return true;
}

bool
vk_create_renderer(struct vk_ctx *ctx,
		   const char *vs_src,
		   unsigned int vs_size,
		   const char *fs_src,
		   unsigned int fs_size,
		   bool enable_depth,
		   bool enable_stencil,
		   struct vk_image_att *color_att,
		   struct vk_image_att *depth_att,
		   struct vk_renderer *renderer)
{
	renderer->renderpass = create_renderpass(ctx, &color_att->props, &depth_att->props);
	if (renderer->renderpass == VK_NULL_HANDLE)
		goto fail;

	create_framebuffer(ctx, color_att, depth_att, renderer);
	if (renderer->fb == VK_NULL_HANDLE)
		goto fail;

	renderer->vs = create_shader_module(ctx, vs_src, vs_size);
	if (renderer->vs == VK_NULL_HANDLE)
		goto fail;

	renderer->fs = create_shader_module(ctx, fs_src, fs_size);
	if (renderer->fs == VK_NULL_HANDLE)
		goto fail;

	create_pipeline(ctx, color_att->props.w, color_att->props.h,
			color_att->props.num_samples, enable_depth, enable_stencil, renderer);

	if (renderer->pipeline == VK_NULL_HANDLE)
		goto fail;

	return true;

fail:
	fprintf(stderr, "Failed to create graphics pipeline.\n");
	vk_destroy_renderer(ctx, renderer);
	return false;
}

void
vk_destroy_renderer(struct vk_ctx *ctx,
		    struct vk_renderer *renderer)
{
	if (renderer->renderpass != VK_NULL_HANDLE)
		vkDestroyRenderPass(ctx->dev, renderer->renderpass, 0);

	if (renderer->vs != VK_NULL_HANDLE)
		vkDestroyShaderModule(ctx->dev, renderer->vs, 0);

	if (renderer->fs != VK_NULL_HANDLE)
		vkDestroyShaderModule(ctx->dev, renderer->fs, 0);

	if (renderer->pipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(ctx->dev, renderer->pipeline, 0);

	if (renderer->fb != VK_NULL_HANDLE)
		vkDestroyFramebuffer(ctx->dev, renderer->fb, 0);

	if (renderer->pipeline_layout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(ctx->dev, renderer->pipeline_layout, 0);
}

bool
vk_create_buffer(struct vk_ctx *ctx,
		 uint32_t sz,
		 VkBufferUsageFlagBits usage,
		 void *pnext,
		 struct vk_buf *bo)
{
	VkBufferCreateInfo buf_info;
	VkMemoryRequirements mem_reqs;

	bo->mobj.mem = VK_NULL_HANDLE;
	bo->buf = VK_NULL_HANDLE;

	/* VkBufferCreateInfo */
	memset(&buf_info, 0, sizeof buf_info);
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.size = sz;
	buf_info.usage = usage;
	buf_info.pNext = pnext;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(ctx->dev, &buf_info, 0, &bo->buf) != VK_SUCCESS)
		goto fail;

	/* allocate buffer */
	vkGetBufferMemoryRequirements(ctx->dev, bo->buf, &mem_reqs);
	/* VK_MEMORY_PROPERTY_HOST_COHERENT_BIT bit specifies that the
	 * host cache management commands vkFlushMappedMemoryRanges and
	 * vkInvalidateMappedMemoryRanges are not needed to flush host
	 * writes to the device or make device writes visible to the
	 * host, respectively. */
	bo->mobj.mem = alloc_memory(ctx, &mem_reqs,
				    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
				    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	if (bo->mobj.mem == VK_NULL_HANDLE)
		goto fail;

	bo->mobj.mem_sz = sz;

	if (vkBindBufferMemory(ctx->dev, bo->buf, bo->mobj.mem, 0) != VK_SUCCESS) {
		fprintf(stderr, "Failed to bind buffer memory.\n");
		goto fail;
	}

	return true;

fail:
	fprintf(stderr, "Failed to allocate buffer.\n");
	vk_destroy_buffer(ctx, bo);
	return false;
}

bool
vk_update_buffer_data(struct vk_ctx *ctx,
		      void *data,
		      uint32_t data_sz,
		      struct vk_buf *bo)
{
	void *map;

	if (vkMapMemory(ctx->dev, bo->mobj.mem, 0, data_sz, 0, &map) != VK_SUCCESS) {
		fprintf(stderr, "Failed to map buffer memory.\n");
		goto fail;
	}

	memcpy(map, data, data_sz);

	vkUnmapMemory(ctx->dev, bo->mobj.mem);
	return true;

fail:
	fprintf(stderr, "Failed to update buffer data. Destroying the buffer.\n");
	vk_destroy_buffer(ctx, bo);

	return false;
}

void
vk_destroy_buffer(struct vk_ctx *ctx,
		  struct vk_buf *bo)
{
	if (bo->buf != VK_NULL_HANDLE)
		vkDestroyBuffer(ctx->dev, bo->buf, 0);

	if (bo->mobj.mem != VK_NULL_HANDLE)
		vkFreeMemory(ctx->dev, bo->mobj.mem, 0);

	bo->mobj.mem_sz = 0;
	bo->buf = VK_NULL_HANDLE;
	bo->mobj.mem = VK_NULL_HANDLE;
}

void
vk_draw(struct vk_ctx *ctx,
	struct vk_buf *vbo,
	struct vk_renderer *renderer,
	float *vk_fb_color,
	uint32_t vk_fb_color_count,
	struct vk_semaphores *semaphores,
	bool has_wait, bool has_signal,
	struct vk_image_att *attachments,
	uint32_t n_attachments,
	float x, float y,
	float w, float h)
{
	VkCommandBufferBeginInfo cmd_begin_info;
	VkRenderPassBeginInfo rp_begin_info;
	VkRect2D rp_area;
	VkClearValue clear_values[2];
	VkSubmitInfo submit_info;
	VkDeviceSize dev_sz = 0;
	VkPipelineStageFlagBits stage_flags;
	struct vk_dims img_size;

	assert(vk_fb_color_count == 4);
	if (has_wait)
		assert(semaphores->gl_frame_done);
	if (has_signal)
		assert(semaphores->vk_frame_ready);

	/* VkCommandBufferBeginInfo */
	memset(&cmd_begin_info, 0, sizeof cmd_begin_info);
	cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	/* VkRect2D render area */
	memset(&rp_area, 0, sizeof rp_area);
	rp_area.extent.width = (uint32_t)w;
	rp_area.extent.height = (uint32_t)h;

	/* VkClearValue */
	memset(&clear_values[0], 0, sizeof clear_values[0]);
	clear_values[0].color.float32[0] = vk_fb_color[0]; /* red */
	clear_values[0].color.float32[1] = vk_fb_color[1]; /* green */
	clear_values[0].color.float32[2] = vk_fb_color[2]; /* blue */
	clear_values[0].color.float32[3] = vk_fb_color[3]; /* alpha */

	memset(&clear_values[1], 0, sizeof clear_values[1]);
	clear_values[1].depthStencil.depth = 1.0;

	/* VkRenderPassBeginInfo */
	memset(&rp_begin_info, 0, sizeof rp_begin_info);
	rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin_info.renderPass = renderer->renderpass;
	rp_begin_info.framebuffer = renderer->fb;
	rp_begin_info.renderArea = rp_area;
	rp_begin_info.clearValueCount = 2;
	rp_begin_info.pClearValues = clear_values;

	/* VkSubmitInfo */
	stage_flags = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;

	memset(&submit_info, 0, sizeof submit_info);
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &ctx->cmd_buf;
	if (has_wait) {
		submit_info.pWaitDstStageMask = &stage_flags;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &semaphores->gl_frame_done;
	}

	if (has_signal) {
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &semaphores->vk_frame_ready;
	}

	vkBeginCommandBuffer(ctx->cmd_buf, &cmd_begin_info);
	vkCmdBeginRenderPass(ctx->cmd_buf, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	viewport.x = x;
	viewport.y = y;
	viewport.width = w;
	viewport.height = h;

	vkCmdSetViewport(ctx->cmd_buf, 0, 1, &viewport);
	vkCmdSetScissor(ctx->cmd_buf, 0, 1, &scissor);

	img_size.w = (float)w;
	img_size.h = (float)h;
	vkCmdPushConstants(ctx->cmd_buf,
			   renderer->pipeline_layout,
			   VK_SHADER_STAGE_FRAGMENT_BIT,
			   0, sizeof (struct vk_dims),
			   &img_size);

	if (vbo) {
		vkCmdBindVertexBuffers(ctx->cmd_buf, 1, 1, &vbo->buf, &dev_sz);
	}
	vkCmdBindPipeline(ctx->cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline);

	vkCmdDraw(ctx->cmd_buf, 4, 1, 0, 0);

	if (attachments) {
		VkImageMemoryBarrier *barriers =
			malloc(n_attachments * sizeof(VkImageMemoryBarrier));
		VkImageMemoryBarrier *barrier = barriers;
		for (uint32_t n = 0; n < n_attachments; n++, barrier++) {
			struct vk_image_att *att = &attachments[n];

			/* Insert barrier to mark ownership transfer. */
			barrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

			bool is_depth =
				get_aspect_from_depth_format(att->props.format) != VK_NULL_HANDLE;

			barrier->oldLayout = is_depth ?
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier->newLayout = VK_IMAGE_LAYOUT_GENERAL;
			barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
			barrier->image = att->obj.img;
			barrier->subresourceRange.aspectMask = is_depth ?
				VK_IMAGE_ASPECT_DEPTH_BIT :
				VK_IMAGE_ASPECT_COLOR_BIT;
			barrier->subresourceRange.baseMipLevel = 0;
			barrier->subresourceRange.levelCount = 1;
			barrier->subresourceRange.baseArrayLayer = 0;
			barrier->subresourceRange.layerCount = 1;
			barrier->srcAccessMask = 0;
			barrier->dstAccessMask = 0;
		}

		vkCmdPipelineBarrier(ctx->cmd_buf,
				     VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				     VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				     0,
				     0, NULL,
				     0, NULL,
				     n_attachments, barriers);
		free(barriers);
	}

	vkCmdEndRenderPass(ctx->cmd_buf);
	vkEndCommandBuffer(ctx->cmd_buf);

	if (vkQueueSubmit(ctx->queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
		fprintf(stderr, "Failed to submit queue.\n");
	}

	/* FIXME */
	if (!semaphores && !has_wait && !has_signal)
		vkQueueWaitIdle(ctx->queue);
}

void
vk_copy_image_to_buffer(struct vk_ctx *ctx,
			struct vk_image_att *src_img,
			struct vk_buf *dst_bo,
			float w, float h)
{
	VkCommandBufferBeginInfo cmd_begin_info;
	VkSubmitInfo submit_info;
	VkImageAspectFlagBits aspect_mask = get_aspect_from_depth_format(src_img->props.format);

	/* VkCommandBufferBeginInfo */
	memset(&cmd_begin_info, 0, sizeof cmd_begin_info);
	cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	memset(&submit_info, 0, sizeof submit_info);
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &ctx->cmd_buf;

	vkBeginCommandBuffer(ctx->cmd_buf, &cmd_begin_info);
	if (src_img->props.end_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && dst_bo) {
		vk_transition_image_layout(src_img,
					   ctx->cmd_buf,
					   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					   VK_QUEUE_FAMILY_EXTERNAL,
					   VK_QUEUE_FAMILY_IGNORED);

		/* copy image to buf */
		VkBufferImageCopy copy_region = {
			.bufferOffset = 0,
			.bufferRowLength = w,
			.bufferImageHeight = h,
			.imageSubresource = {
				.aspectMask = aspect_mask ? aspect_mask : VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = { w, h, 1 }
                };

		vkCmdCopyImageToBuffer(ctx->cmd_buf,
				       src_img->obj.img,
				       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				       dst_bo->buf, 1, &copy_region);

		vk_transition_image_layout(src_img,
					   ctx->cmd_buf,
					   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					   VK_QUEUE_FAMILY_EXTERNAL,
					   VK_QUEUE_FAMILY_IGNORED);

		VkBufferMemoryBarrier write_finish_buffer_memory_barrier = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_HOST_READ_BIT,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.buffer = dst_bo->buf,
			.offset = 0,
			.size = VK_WHOLE_SIZE
		};

		vkCmdPipelineBarrier(ctx->cmd_buf,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_HOST_BIT,
				(VkDependencyFlags) 0, 0, NULL,
				1, &write_finish_buffer_memory_barrier,
				0, NULL);
	}
	vkEndCommandBuffer(ctx->cmd_buf);

	if (vkQueueSubmit(ctx->queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
		fprintf(stderr, "Failed to submit queue.\n");
	}
	vkQueueWaitIdle(ctx->queue);
}

bool
vk_create_semaphores(struct vk_ctx *ctx,
		     struct vk_semaphores *semaphores)
{
	VkSemaphoreCreateInfo sema_info;
	VkExportSemaphoreCreateInfo exp_sema_info;

	/* VkExportSemaphoreCreateInfo */
	memset(&exp_sema_info, 0, sizeof exp_sema_info);
	exp_sema_info.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
	exp_sema_info.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;

	/* VkSemaphoreCreateInfo */
	memset(&sema_info, 0, sizeof sema_info);
	sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	sema_info.pNext = &exp_sema_info;

	if (vkCreateSemaphore(ctx->dev, &sema_info, 0, &semaphores->vk_frame_ready) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create semaphore vk_frame_ready.\n");
		return false;
	}

	if (vkCreateSemaphore(ctx->dev, &sema_info, 0, &semaphores->gl_frame_done) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create semaphore gl_frame_done.\n");
		return false;
	}

	return true;
}

void
vk_destroy_semaphores(struct vk_ctx *ctx,
		      struct vk_semaphores *semaphores)
{
	if (semaphores->vk_frame_ready)
		vkDestroySemaphore(ctx->dev, semaphores->vk_frame_ready, 0);
	if (semaphores->gl_frame_done)
		vkDestroySemaphore(ctx->dev, semaphores->gl_frame_done, 0);
}

void
vk_transition_image_layout(struct vk_image_att *img_att,
			   VkCommandBuffer cmd_buf,
			   VkImageLayout old_layout,
			   VkImageLayout new_layout,
			   uint32_t src_queue_fam_idx,
			   uint32_t dst_queue_fam_idx)
{
	VkImageMemoryBarrier barrier;
	struct vk_image_props props = img_att->props;
	VkImageAspectFlagBits aspect_mask = get_aspect_from_depth_format(props.format);

	memset(&barrier, 0, sizeof barrier);
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = get_access_mask(old_layout);
	barrier.dstAccessMask = get_access_mask(new_layout);
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = src_queue_fam_idx;
	barrier.dstQueueFamilyIndex = dst_queue_fam_idx;
	barrier.image = img_att->obj.img;
	barrier.subresourceRange.aspectMask = aspect_mask ? aspect_mask : VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(cmd_buf,
			     get_pipeline_stage_flags(old_layout),
			     get_pipeline_stage_flags(new_layout),
			     0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);
}

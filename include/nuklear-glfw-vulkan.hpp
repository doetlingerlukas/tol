#ifndef NK_VULKAN_GLFW_H_
#define NK_VULKAN_GLFW_H_

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_VULKAN_IMPLEMENTATION

#include <stdarg.h>
#include "nuklear.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

enum nk_glfw_init_state{
    NK_GLFW3_DEFAULT=0,
    NK_GLFW3_INSTALL_CALLBACKS
};

NK_API struct nk_context*   nk_glfw3_init(GLFWwindow *win, vk::Device logical_device, vk::PhysicalDevice physical_device, vk::Queue graphics_queue, uint32_t graphics_queue_index, std::vector<vk::Framebuffer> framebuffers, vk::Format color_format, vk::Format depth_format, enum nk_glfw_init_state);
NK_API void                 nk_glfw3_shutdown(void);
NK_API void                 nk_glfw3_font_stash_begin(struct nk_font_atlas **atlas);
NK_API void                 nk_glfw3_font_stash_end(void);
NK_API void                 nk_glfw3_new_frame();
NK_API vk::Semaphore          nk_glfw3_render(enum nk_anti_aliasing AA, uint32_t buffer_index, vk::Semaphore wait_semaphore);

NK_API void                 nk_glfw3_device_destroy(void);
NK_API void                 nk_glfw3_device_create(vk::Device, vk::PhysicalDevice, vk::Queue graphics_queue, uint32_t graphics_queue_index, std::vector<vk::Framebuffer> framebuffers, vk::Format, vk::Format);

NK_API void                 nk_glfw3_char_callback(GLFWwindow *win, unsigned int codepoint);
NK_API void                 nk_gflw3_scroll_callback(GLFWwindow *win, double xoff, double yoff);
NK_API void                 nk_glfw3_mouse_button_callback(GLFWwindow *win, int button, int action, int mods);

#endif

#ifdef NK_GLFW_VULKAN_IMPLEMENTATION

// INCLUDES INLINED
#include "nuklear.frag.h"
#include "nuklear.vert.h"

#ifndef NK_GLFW_TEXT_MAX
#define NK_GLFW_TEXT_MAX 256
#endif
#ifndef NK_GLFW_DOUBLE_CLICK_LO
#define NK_GLFW_DOUBLE_CLICK_LO 0.02
#endif
#ifndef NK_GLFW_DOUBLE_CLICK_HI
#define NK_GLFW_DOUBLE_CLICK_HI 0.2
#endif

#define VK_COLOR_COMPONENT_MASK_RGBA VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_INDEX_BUFFER 128 * 1024

struct Mat4f {
    float m[16];
};

struct nk_vulkan_adapter {
    struct nk_buffer cmds;
    struct nk_draw_null_texture null;
    vk::Device logical_device;
    vk::PhysicalDevice physical_device;
    vk::Queue graphics_queue;
    uint32_t graphics_queue_index;
    std::vector<vk::Framebuffer> framebuffers;
    vk::Format color_format;
    vk::Format depth_format;
    vk::Semaphore render_completed;
    vk::Sampler font_tex;
    vk::Image font_image;
    vk::ImageView font_image_view;
    vk::DeviceMemory font_memory;
    vk::PipelineLayout pipeline_layout;
    vk::RenderPass render_pass;
    vk::Pipeline pipeline;
    vk::Buffer vertex_buffer;
    vk::DeviceMemory vertex_memory;
    vk::Buffer index_buffer;
    vk::DeviceMemory index_memory;
    vk::Buffer uniform_buffer;
    vk::DeviceMemory uniform_memory;
    vk::CommandPool command_pool;
    std::vector<vk::CommandBuffer> command_buffers; // currently always length 1
    vk::DescriptorPool descriptor_pool;
    vk::DescriptorSetLayout descriptor_set_layout;
    vk::DescriptorSet descriptor_set;
};

struct nk_glfw_vertex {
    float position[2];
    float uv[2];
    nk_byte col[4];
};

static struct nk_glfw {
    GLFWwindow *win;
    int width, height;
    int display_width, display_height;
    struct nk_vulkan_adapter adapter;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_vec2 fb_scale;
    unsigned int text[NK_GLFW_TEXT_MAX];
    int text_len;
    struct nk_vec2 scroll;
    double last_button_click;
    int is_double_click_down;
    struct nk_vec2 double_click_pos;
} glfw;

vk::PipelineShaderStageCreateInfo create_shader(struct nk_vulkan_adapter* adapter, unsigned char* spv_shader, uint32_t size, vk::ShaderStageFlagBits stage_bit) {
    vk::ShaderModuleCreateInfo create_info;
    create_info.codeSize = size;
    create_info.pCode = (const uint32_t*) spv_shader;

    auto module = adapter->logical_device.createShaderModule(create_info);

    vk::PipelineShaderStageCreateInfo shader_info;
    shader_info.stage = stage_bit;
    shader_info.module = module;
    shader_info.pName = "main";

    return shader_info;
}

void prepare_descriptor_pool(struct nk_vulkan_adapter* adapter) {
    vk::DescriptorPoolSize pool_sizes[2];
    pool_sizes[0].type = vk::DescriptorType::eUniformBuffer;
    pool_sizes[0].descriptorCount = 1;
    pool_sizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    pool_sizes[1].descriptorCount = 1;

    vk::DescriptorPoolCreateInfo pool_info;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = 2;
    pool_info.pPoolSizes = pool_sizes;

    adapter->descriptor_pool = adapter->logical_device.createDescriptorPool(pool_info);
}

void prepare_descriptor_set_layout(struct nk_vulkan_adapter* adapter) {
    vk::DescriptorSetLayoutBinding bindings[2];
    bindings[0].binding = 0;
    bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex;
    bindings[1].binding = 1;
    bindings[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutCreateInfo descriptor_set_info;
    descriptor_set_info.bindingCount = 2;
    descriptor_set_info.pBindings = bindings;

    adapter->descriptor_set_layout = adapter->logical_device.createDescriptorSetLayout(descriptor_set_info);
}

void prepare_descriptor_set(struct nk_vulkan_adapter* adapter) {
    vk::DescriptorSetAllocateInfo allocate_info;
    allocate_info.descriptorPool = adapter->descriptor_pool;
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &adapter->descriptor_set_layout;

    adapter->descriptor_set = adapter->logical_device.allocateDescriptorSets(allocate_info)[0];
}

void update_write_descriptor_sets(struct nk_vulkan_adapter* adapter) {
    vk::DescriptorBufferInfo buffer_info;
    buffer_info.buffer = adapter->uniform_buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(struct Mat4f);

    vk::DescriptorImageInfo image_info;
    image_info.sampler = adapter->font_tex;
    image_info.imageView = adapter->font_image_view;
    image_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::WriteDescriptorSet descriptor_writes[2];
    descriptor_writes[0].dstSet = adapter->descriptor_set;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].dstArrayElement = 0;
    descriptor_writes[0].descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].pBufferInfo = &buffer_info;

    uint32_t descriptor_write_count = 2;
    descriptor_writes[1].dstSet = adapter->descriptor_set;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].dstArrayElement = 0;
    descriptor_writes[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].pImageInfo = &image_info;

    adapter->logical_device.updateDescriptorSets(descriptor_write_count, descriptor_writes, 0, VK_NULL_HANDLE);
}

void prepare_pipeline_layout(struct nk_vulkan_adapter* adapter) {
    vk::PipelineLayoutCreateInfo pipeline_layout_info;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &adapter->descriptor_set_layout;

    adapter->pipeline_layout = adapter->logical_device.createPipelineLayout(pipeline_layout_info);
}

void prepare_pipeline(struct nk_vulkan_adapter* adapter) {
    vk::PipelineInputAssemblyStateCreateInfo input_assembly_state;
    input_assembly_state.topology = vk::PrimitiveTopology::eTriangleList;
    input_assembly_state.primitiveRestartEnable = VK_FALSE;

    vk::PipelineRasterizationStateCreateInfo rasterization_state;
    rasterization_state.polygonMode = vk::PolygonMode::eFill;
    rasterization_state.cullMode = vk::CullModeFlagBits::eBack;
    rasterization_state.frontFace = vk::FrontFace::eClockwise;
    rasterization_state.lineWidth = 1.0f;

    vk::PipelineColorBlendAttachmentState attachment_state;
    attachment_state.blendEnable = VK_TRUE;
    attachment_state.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    attachment_state.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    attachment_state.colorBlendOp = vk::BlendOp::eAdd;
    attachment_state.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    attachment_state.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    attachment_state.alphaBlendOp = vk::BlendOp::eAdd;
    attachment_state.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    vk::PipelineColorBlendStateCreateInfo color_blend_state;
    color_blend_state.attachmentCount = 1;
    color_blend_state.pAttachments = &attachment_state;

    vk::PipelineDepthStencilStateCreateInfo depth_stencil_state;
    depth_stencil_state.depthTestEnable = VK_TRUE;
    depth_stencil_state.depthWriteEnable = VK_TRUE;
    depth_stencil_state.depthCompareOp = vk::CompareOp::eLessOrEqual;

    vk::PipelineViewportStateCreateInfo viewport_state;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    vk::PipelineMultisampleStateCreateInfo multisample_state;
    multisample_state.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::DynamicState dynamic_states[2] = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamic_state;
    dynamic_state.dynamicStateCount = 2;
    dynamic_state.pDynamicStates = dynamic_states;

    vk::PipelineShaderStageCreateInfo shader_stages[2] = {
        create_shader(adapter, shaders_nuklear_vert_spv, shaders_nuklear_vert_spv_len, vk::ShaderStageFlagBits::eVertex),
        create_shader(adapter, shaders_nuklear_frag_spv, shaders_nuklear_frag_spv_len, vk::ShaderStageFlagBits::eFragment)
    };

    vk::VertexInputBindingDescription vertex_input_info[1];
    vertex_input_info[0].binding = 0;
    vertex_input_info[0].stride = sizeof(struct nk_glfw_vertex);
    vertex_input_info[0].inputRate = vk::VertexInputRate::eVertex;

    vk::VertexInputAttributeDescription vertex_attribute_description[3];
    vertex_attribute_description[0].location = 0;
    vertex_attribute_description[0].binding = 0;
    vertex_attribute_description[0].format = vk::Format::eR32G32Sfloat;
    vertex_attribute_description[0].offset = static_cast<uint32_t>(NK_OFFSETOF(struct nk_glfw_vertex, position));
    vertex_attribute_description[1].location = 1;
    vertex_attribute_description[1].binding = 0;
    vertex_attribute_description[1].format = vk::Format::eR32G32Sfloat;
    vertex_attribute_description[1].offset = static_cast<uint32_t>(NK_OFFSETOF(struct nk_glfw_vertex, uv));
    vertex_attribute_description[2].location = 2;
    vertex_attribute_description[2].binding = 0;
    vertex_attribute_description[2].format = vk::Format::eR8G8B8A8Uint;
    vertex_attribute_description[2].offset = static_cast<uint32_t>(NK_OFFSETOF(struct nk_glfw_vertex, col));

    vk::PipelineVertexInputStateCreateInfo vertex_input;
    vertex_input.vertexBindingDescriptionCount = 1;
    vertex_input.pVertexBindingDescriptions = vertex_input_info;
    vertex_input.vertexAttributeDescriptionCount = 3;
    vertex_input.pVertexAttributeDescriptions = vertex_attribute_description;

    vk::GraphicsPipelineCreateInfo pipeline_info;
    pipeline_info.flags = {};
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input;
    pipeline_info.pInputAssemblyState = &input_assembly_state;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterization_state;
    pipeline_info.pMultisampleState = &multisample_state;
    pipeline_info.pDepthStencilState = &depth_stencil_state;
    pipeline_info.pColorBlendState = &color_blend_state;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.layout = adapter->pipeline_layout;
    pipeline_info.renderPass = adapter->render_pass;
    pipeline_info.basePipelineHandle = nullptr;
    pipeline_info.basePipelineIndex = -1;

    adapter->pipeline = adapter->logical_device.createGraphicsPipelines(nullptr, pipeline_info).value[0];

    adapter->logical_device.destroyShaderModule(shader_stages[0].module);
    adapter->logical_device.destroyShaderModule(shader_stages[1].module);
}

void prepare_render_pass(struct nk_vulkan_adapter* adapter) {
    vk::AttachmentDescription attachments[1];
    attachments[0].format = adapter->color_format;
    attachments[0].samples = vk::SampleCountFlagBits::e1;
    attachments[0].loadOp = vk::AttachmentLoadOp::eLoad;
    attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[0].initialLayout = vk::ImageLayout::eUndefined;
    attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference color_reference;
    color_reference.attachment = 0;
    color_reference.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDependency subpass_dependencies[1];
    subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependencies[0].dstSubpass = 0;
    subpass_dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    subpass_dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    subpass_dependencies[0].srcAccessMask = {};
    subpass_dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

    vk::SubpassDescription subpass_description;
    subpass_description.flags = {};
    subpass_description.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass_description.inputAttachmentCount = 0;
    subpass_description.pInputAttachments = NULL;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &color_reference;
    subpass_description.pResolveAttachments = NULL;
    subpass_description.pDepthStencilAttachment = VK_NULL_HANDLE;
    subpass_description.preserveAttachmentCount = 0;
    subpass_description.pPreserveAttachments = NULL;

    vk::RenderPassCreateInfo render_pass_info;
    render_pass_info.pNext = NULL;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = attachments;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass_description;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = subpass_dependencies;

    adapter->render_pass = adapter->logical_device.createRenderPass(render_pass_info);
}

void prepare_command_buffers(struct nk_vulkan_adapter* adapter) {
    vk::CommandPoolCreateInfo create_info;
    create_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    create_info.queueFamilyIndex = adapter->graphics_queue_index;

    adapter->command_pool = adapter->logical_device.createCommandPool(create_info);

    vk::CommandBufferAllocateInfo allocate_info;
    allocate_info.commandPool = adapter->command_pool;
    allocate_info.level = vk::CommandBufferLevel::ePrimary;
    allocate_info.commandBufferCount = 1;

    adapter->command_buffers = adapter->logical_device.allocateCommandBuffers(allocate_info);
}

void prepare_semaphores(struct nk_vulkan_adapter *adapter) {
  vk::SemaphoreCreateInfo semaphore_info;
  adapter->render_completed = adapter->logical_device.createSemaphore(semaphore_info);
}

uint32_t find_memory_index(vk::PhysicalDevice physical_device, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    auto mem_properties = physical_device.getMemoryProperties();

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    assert(0);
    return 0;
}

void create_buffer_and_memory(struct nk_vulkan_adapter* adapter, vk::Buffer* buffer, vk::BufferUsageFlags usage, vk::DeviceMemory* memory, vk::DeviceSize size) {
    vk::BufferCreateInfo buffer_info;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = vk::SharingMode::eExclusive;

    *buffer = adapter->logical_device.createBuffer(buffer_info);

    auto mem_reqs = adapter->logical_device.getBufferMemoryRequirements(*buffer);

    vk::MemoryAllocateInfo alloc_info;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = find_memory_index(adapter->physical_device, mem_reqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    *memory = adapter->logical_device.allocateMemory(alloc_info);
    adapter->logical_device.bindBufferMemory(*buffer, *memory, 0);
}

NK_API void
nk_glfw3_device_create(vk::Device logical_device, vk::PhysicalDevice physical_device, vk::Queue graphics_queue, uint32_t graphics_queue_index, std::vector<vk::Framebuffer> framebuffers, vk::Format color_format, vk::Format depth_format) {
    struct nk_vulkan_adapter *adapter = &glfw.adapter;
    nk_buffer_init_default(&adapter->cmds);
    adapter->logical_device = logical_device;
    adapter->physical_device = physical_device;
    adapter->graphics_queue = graphics_queue,
    adapter->graphics_queue_index = graphics_queue_index;
    adapter->framebuffers = framebuffers;
    adapter->color_format = color_format;
    adapter->depth_format = depth_format;

    prepare_semaphores(adapter);
    prepare_render_pass(adapter);

    create_buffer_and_memory(adapter, &adapter->vertex_buffer, vk::BufferUsageFlagBits::eVertexBuffer, &adapter->vertex_memory, MAX_VERTEX_BUFFER);
    create_buffer_and_memory(adapter, &adapter->index_buffer, vk::BufferUsageFlagBits::eIndexBuffer, &adapter->index_memory, MAX_INDEX_BUFFER);
    create_buffer_and_memory(adapter, &adapter->uniform_buffer, vk::BufferUsageFlagBits::eUniformBuffer, &adapter->uniform_memory, sizeof(struct Mat4f));

    prepare_descriptor_pool(adapter);
    prepare_descriptor_set_layout(adapter);
    prepare_descriptor_set(adapter);
    prepare_pipeline_layout(adapter);
    prepare_pipeline(adapter);

    prepare_command_buffers(adapter);
}

NK_API void
nk_glfw3_device_destroy(void)
{
    struct nk_vulkan_adapter *adapter = &glfw.adapter;
    nk_buffer_free(&adapter->cmds);
}

NK_API void
nk_glfw3_char_callback(GLFWwindow *win, unsigned int codepoint)
{
    (void)win;
    if (glfw.text_len < NK_GLFW_TEXT_MAX)
        glfw.text[glfw.text_len++] = codepoint;
}

NK_API void
nk_gflw3_scroll_callback(GLFWwindow *win, double xoff, double yoff)
{
    (void)win; (void)xoff;
    glfw.scroll.x += (float)xoff;
    glfw.scroll.y += (float)yoff;
}

NK_API void
nk_glfw3_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double x, y;
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    glfwGetCursorPos(window, &x, &y);
    if (action == GLFW_PRESS)  {
        double dt = glfwGetTime() - glfw.last_button_click;
        if (dt > NK_GLFW_DOUBLE_CLICK_LO && dt < NK_GLFW_DOUBLE_CLICK_HI) {
            glfw.is_double_click_down = nk_true;
            glfw.double_click_pos = nk_vec2((float) x, (float) y);
        }
        glfw.last_button_click = glfwGetTime();
    } else glfw.is_double_click_down = nk_false;
}

NK_INTERN void
nk_glfw3_clipbard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    const char *text = glfwGetClipboardString(glfw.win);
    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
    (void)usr;
}

NK_INTERN void
nk_glfw3_clipbard_copy(nk_handle usr, const char *text, int len)
{
    char *str = 0;
    (void)usr;
    if (!len) return;
    str = (char*)malloc((size_t)len+1);
    if (!str) return;
    memcpy(str, text, (size_t)len);
    str[len] = '\0';
    glfwSetClipboardString(glfw.win, str);
    free(str);
}

NK_API struct nk_context*
nk_glfw3_init(GLFWwindow *win, vk::Device logical_device, vk::PhysicalDevice physical_device, vk::Queue graphics_queue, uint32_t graphics_queue_index, std::vector<vk::Framebuffer> framebuffers, vk::Format color_format, vk::Format depth_format, enum nk_glfw_init_state init_state)
{
    glfw.win = win;
    if (init_state == NK_GLFW3_INSTALL_CALLBACKS) {
        glfwSetScrollCallback(win, nk_gflw3_scroll_callback);
        glfwSetCharCallback(win, nk_glfw3_char_callback);
        glfwSetMouseButtonCallback(win, nk_glfw3_mouse_button_callback);
    }
    nk_init_default(&glfw.ctx, 0);
    glfw.ctx.clip.copy = nk_glfw3_clipbard_copy;
    glfw.ctx.clip.paste = nk_glfw3_clipbard_paste;
    glfw.ctx.clip.userdata = nk_handle_ptr(0);
    glfw.last_button_click = 0;
    nk_glfw3_device_create(logical_device, physical_device, graphics_queue, graphics_queue_index, framebuffers, color_format, depth_format);

    glfw.is_double_click_down = nk_false;
    glfw.double_click_pos = nk_vec2(0, 0);

    return &glfw.ctx;
}

NK_INTERN void
nk_glfw3_device_upload_atlas(const void *image, int width, int height)
{
    struct nk_vulkan_adapter *adapter = &glfw.adapter;

    auto device = adapter->logical_device;

    vk::ImageCreateInfo image_info;
    image_info.imageType = vk::ImageType::e2D;
    image_info.format = vk::Format::eR8G8B8A8Unorm;
    image_info.extent.width = static_cast<uint32_t>(width);
    image_info.extent.height = static_cast<uint32_t>(height);
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = vk::SampleCountFlagBits::e1;
    image_info.tiling = vk::ImageTiling::eOptimal;
    image_info.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    image_info.sharingMode = vk::SharingMode::eExclusive;
    image_info.initialLayout = vk::ImageLayout::eUndefined;

    adapter->font_image = device.createImage(image_info);

    vk::MemoryRequirements mem_reqs = device.getImageMemoryRequirements(adapter->font_image);
    vk::MemoryAllocateInfo alloc_info;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = find_memory_index(adapter->physical_device, mem_reqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

    adapter->font_memory = device.allocateMemory(alloc_info);
    device.bindImageMemory(adapter->font_image, adapter->font_memory, 0);

    struct {
			vk::DeviceMemory memory;
			vk::Buffer buffer;
    } staging_buffer;

    vk::BufferCreateInfo buffer_info;
    buffer_info.size = alloc_info.allocationSize;
    buffer_info.usage = vk::BufferUsageFlagBits::eTransferSrc;
    buffer_info.sharingMode = vk::SharingMode::eExclusive;

    staging_buffer.buffer = device.createBuffer(buffer_info);
    mem_reqs = device.getBufferMemoryRequirements(staging_buffer.buffer);

    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = find_memory_index(adapter->physical_device, mem_reqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    staging_buffer.memory = device.allocateMemory(alloc_info);
    device.bindBufferMemory(staging_buffer.buffer, staging_buffer.memory, 0);

    void* data = device.mapMemory(staging_buffer.memory, 0, alloc_info.allocationSize, {});
    memcpy(data, image, width * height * 4);
    device.unmapMemory(staging_buffer.memory);

    // use the same command buffer as for render as we are regenerating the buffer during render anyway
    vk::CommandBufferBeginInfo begin_info;

    // TODO: kill array
    vk::CommandBuffer command_buffer(adapter->command_buffers[0]);
    command_buffer.begin(begin_info);

    vk::ImageMemoryBarrier image_memory_barrier;
    image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    image_memory_barrier.oldLayout = vk::ImageLayout::eUndefined;
    image_memory_barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.image = adapter->font_image;
    vk::ImageSubresourceRange subresource_range;
    subresource_range.aspectMask = vk::ImageAspectFlagBits::eColor;
    subresource_range.levelCount = 1;
    subresource_range.layerCount = 1;
    image_memory_barrier.subresourceRange = subresource_range;

    command_buffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eTopOfPipe,
      vk::PipelineStageFlagBits::eTransfer,
      {},
      {},
      {},
      image_memory_barrier
    );

    vk::BufferImageCopy buffer_copy_region;
    vk::ImageSubresourceLayers subresource;
    subresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    subresource.mipLevel = 0;
    subresource.layerCount = 1;
    buffer_copy_region.imageSubresource = subresource;
    buffer_copy_region.imageExtent.width = static_cast<uint32_t>(width);
    buffer_copy_region.imageExtent.height = static_cast<uint32_t>(height);
    buffer_copy_region.imageExtent.depth = 1;

    command_buffer.copyBufferToImage(
      staging_buffer.buffer,
      adapter->font_image,
      vk::ImageLayout::eTransferDstOptimal,
      buffer_copy_region
    );

    vk::ImageMemoryBarrier image_shader_memory_barrier;
    image_shader_memory_barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    image_shader_memory_barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    image_shader_memory_barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    image_shader_memory_barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    image_shader_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_shader_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_shader_memory_barrier.image = adapter->font_image;
    image_shader_memory_barrier.subresourceRange = subresource_range;

	  command_buffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eFragmentShader,
      {},
      {},
      {},
      image_shader_memory_barrier
    );

    command_buffer.end();

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffer;

    vk::Queue queue(adapter->graphics_queue);

	  queue.submit(submitInfo);
    queue.waitIdle();

    device.free(staging_buffer.memory);
    device.destroy(staging_buffer.buffer);

    vk::ImageViewCreateInfo image_view_info;
    image_view_info.image = adapter->font_image;
    image_view_info.viewType = vk::ImageViewType::e2D;
    image_view_info.format = image_info.format;
    image_view_info.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    adapter->font_image_view = device.createImageView(image_view_info);

    vk::SamplerCreateInfo sampler_info;
    sampler_info.magFilter = vk::Filter::eLinear;
    sampler_info.minFilter = vk::Filter::eLinear;
    sampler_info.mipmapMode = vk::SamplerMipmapMode::eLinear;
    sampler_info.addressModeU = vk::SamplerAddressMode::eRepeat;
    sampler_info.addressModeV = vk::SamplerAddressMode::eRepeat;
    sampler_info.addressModeW = vk::SamplerAddressMode::eRepeat;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.maxAnisotropy = 1.0;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = vk::CompareOp::eAlways;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;
    sampler_info.borderColor = vk::BorderColor::eFloatOpaqueBlack;

    adapter->font_tex = device.createSampler(sampler_info);
}

NK_API void
nk_glfw3_font_stash_begin(struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(&glfw.atlas);
    nk_font_atlas_begin(&glfw.atlas);
    *atlas = &glfw.atlas;
}

NK_API void
nk_glfw3_font_stash_end(void)
{
    struct nk_vulkan_adapter* dev = &glfw.adapter;

    const void *image; int w, h;
    image = nk_font_atlas_bake(&glfw.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    nk_glfw3_device_upload_atlas(image, w, h);
    nk_font_atlas_end(&glfw.atlas, nk_handle_ptr(dev->font_tex), &glfw.adapter.null);
    if (glfw.atlas.default_font) {
        nk_style_set_font(&glfw.ctx, &glfw.atlas.default_font->handle);
    }

    update_write_descriptor_sets(dev);
}

NK_API void
nk_glfw3_new_frame()
{
    int i;
    double x, y;
    struct nk_context *ctx = &glfw.ctx;
    struct GLFWwindow *win = glfw.win;

    glfwGetWindowSize(win, &glfw.width, &glfw.height);
    glfwGetFramebufferSize(win, &glfw.display_width, &glfw.display_height);
    glfw.fb_scale.x = (float)glfw.display_width/(float)glfw.width;
    glfw.fb_scale.y = (float)glfw.display_height/(float)glfw.height;

    nk_input_begin(ctx);
    for (i = 0; i < glfw.text_len; ++i)
        nk_input_unicode(ctx, glfw.text[i]);

#if NK_GLFW_GL3_MOUSE_GRABBING
    /* optional grabbing behavior */
    if (ctx->input.mouse.grab)
        glfwSetInputMode(glfw.win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    else if (ctx->input.mouse.ungrab)
        glfwSetInputMode(glfw.win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#endif

    nk_input_key(ctx, NK_KEY_DEL, glfwGetKey(win, GLFW_KEY_DELETE) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_ENTER, glfwGetKey(win, GLFW_KEY_ENTER) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_TAB, glfwGetKey(win, GLFW_KEY_TAB) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_BACKSPACE, glfwGetKey(win, GLFW_KEY_BACKSPACE) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_UP, glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_DOWN, glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_TEXT_START, glfwGetKey(win, GLFW_KEY_HOME) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_TEXT_END, glfwGetKey(win, GLFW_KEY_END) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_START, glfwGetKey(win, GLFW_KEY_HOME) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_END, glfwGetKey(win, GLFW_KEY_END) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_DOWN, glfwGetKey(win, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_UP, glfwGetKey(win, GLFW_KEY_PAGE_UP) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SHIFT, glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS||
                                    glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
        nk_input_key(ctx, NK_KEY_COPY, glfwGetKey(win, GLFW_KEY_C) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_PASTE, glfwGetKey(win, GLFW_KEY_V) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_CUT, glfwGetKey(win, GLFW_KEY_X) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_UNDO, glfwGetKey(win, GLFW_KEY_Z) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_REDO, glfwGetKey(win, GLFW_KEY_R) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_START, glfwGetKey(win, GLFW_KEY_B) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_END, glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS);
    } else {
        nk_input_key(ctx, NK_KEY_LEFT, glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_RIGHT, glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_COPY, 0);
        nk_input_key(ctx, NK_KEY_PASTE, 0);
        nk_input_key(ctx, NK_KEY_CUT, 0);
        nk_input_key(ctx, NK_KEY_SHIFT, 0);
    }

    glfwGetCursorPos(win, &x, &y);
    nk_input_motion(ctx, (int)x, (int)y);
#if NK_GLFW_GL3_MOUSE_GRABBING
    if (ctx->input.mouse.grabbed) {
        glfwSetCursorPos(glfw.win, ctx->input.mouse.prev.x, ctx->input.mouse.prev.y);
        ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
        ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
    }
#endif
    nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    nk_input_button(ctx, NK_BUTTON_DOUBLE, (int) glfw.double_click_pos.x, (int) glfw.double_click_pos.y, glfw.is_double_click_down);
    nk_input_scroll(ctx, glfw.scroll);
    nk_input_end(&glfw.ctx);
    glfw.text_len = 0;
    glfw.scroll = nk_vec2(0,0);
}

NK_API
vk::Semaphore nk_glfw3_render(enum nk_anti_aliasing AA, uint32_t buffer_index, vk::Semaphore wait_semaphore) {
    struct nk_vulkan_adapter *adapter = &glfw.adapter;
    struct nk_buffer vbuf, ebuf;

    auto device = adapter->logical_device;
    vk::Queue queue(adapter->graphics_queue);

    struct Mat4f projection = {
        .m = {
            2.0f, 0.0f, 0.0f, 0.0f,
            0.0f, -2.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 1.0f
        },
    };
    projection.m[0] /= glfw.width;
    projection.m[5] /= glfw.height;

    void* data = device.mapMemory(adapter->uniform_memory, 0, sizeof(projection), {});
    memcpy(data, &projection, sizeof(projection));
    device.unmapMemory(adapter->uniform_memory);

    vk::CommandBufferBeginInfo begin_info;

    uint32_t display_width_u = glfw.display_width;
    uint32_t display_height_u = glfw.display_height;
    float display_width_f = glfw.display_width;
    float display_height_f = glfw.display_height;

    vk::RenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.renderPass = adapter->render_pass;
    renderPassBeginInfo.framebuffer = adapter->framebuffers[buffer_index];
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = display_width_u;
    renderPassBeginInfo.renderArea.extent.height = display_height_u;
    renderPassBeginInfo.clearValueCount = 0;
    renderPassBeginInfo.pClearValues = VK_NULL_HANDLE;

    vk::CommandBuffer command_buffer(adapter->command_buffers[0]);

    command_buffer.begin(begin_info);
    command_buffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport;
    viewport.width = display_width_f;
    viewport.height = display_height_f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    command_buffer.setViewport(0, viewport);

    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, adapter->pipeline);
    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vk::PipelineLayout(adapter->pipeline_layout), 0, vk::DescriptorSet(adapter->descriptor_set), {});
    {
        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;

        /* load draw vertices & elements directly into vertex + element buffer */
        void* vertices = device.mapMemory(adapter->vertex_memory, 0, MAX_VERTEX_BUFFER, {});
        void* elements = device.mapMemory(adapter->index_memory, 0, MAX_INDEX_BUFFER, {});
        {
            /* fill convert configuration */
            struct nk_convert_config config;
            static const struct nk_draw_vertex_layout_element vertex_layout[] = {
                {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, position)},
                {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, uv)},
                {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_glfw_vertex, col)},
                {NK_VERTEX_LAYOUT_END}
            };
            NK_MEMSET(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(struct nk_glfw_vertex);
            config.vertex_alignment = NK_ALIGNOF(struct nk_glfw_vertex);
            config.null = adapter->null;
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = AA;
            config.line_AA = AA;

            /* setup buffers to load vertices and elements */
            nk_buffer_init_fixed(&vbuf, vertices, (size_t)MAX_VERTEX_BUFFER);
            nk_buffer_init_fixed(&ebuf, elements, (size_t)MAX_INDEX_BUFFER);
            nk_convert(&glfw.ctx, &adapter->cmds, &vbuf, &ebuf, &config);
        }
        device.unmapMemory(adapter->vertex_memory);
        device.unmapMemory(adapter->index_memory);

        /* iterate over and execute each draw command */
        vk::DeviceSize doffset = 0;
        command_buffer.bindVertexBuffers(0, vk::Buffer(adapter->vertex_buffer), doffset);
        command_buffer.bindIndexBuffer(vk::Buffer(adapter->index_buffer), 0, vk::IndexType::eUint16);

        uint32_t index_offset = 0;
        nk_draw_foreach(cmd, &glfw.ctx, &adapter->cmds)
        {
            if (!cmd->elem_count) continue;

            vk::Rect2D scissor;
            scissor.offset.x = static_cast<int32_t>(max(cmd->clip_rect.x * glfw.fb_scale.x, 0));
            scissor.offset.y = static_cast<int32_t>(max(cmd->clip_rect.y * glfw.fb_scale.y, 0));
            scissor.extent.width = static_cast<uint32_t>(cmd->clip_rect.w * glfw.fb_scale.x);
            scissor.extent.height = static_cast<uint32_t>(cmd->clip_rect.h * glfw.fb_scale.y);
            command_buffer.setScissor(0, scissor);
            command_buffer.drawIndexed(cmd->elem_count, 1, index_offset, 0, 0);
            index_offset += cmd->elem_count;
        }
        nk_clear(&glfw.ctx);
    }

    command_buffer.endRenderPass();
    command_buffer.end();

    vk::PipelineStageFlags wait_stages[1] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::SubmitInfo submit_info;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &wait_semaphore;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &adapter->render_completed;

    queue.submit(submit_info);
    queue.waitIdle();

    return adapter->render_completed;
}


NK_API
void nk_glfw3_shutdown(void)
{
    struct nk_vulkan_adapter *adapter = &glfw.adapter;
    adapter->logical_device.free(adapter->command_pool, adapter->command_buffers);
    adapter->logical_device.destroy(adapter->render_completed);

    adapter->logical_device.free(adapter->vertex_memory);
    adapter->logical_device.free(adapter->index_memory);
    adapter->logical_device.free(adapter->uniform_memory);
    adapter->logical_device.free(adapter->font_memory);

    adapter->logical_device.destroy(adapter->vertex_buffer);
    adapter->logical_device.destroy(adapter->index_buffer);
    adapter->logical_device.destroy(adapter->uniform_buffer);
    adapter->logical_device.destroy(adapter->font_image);

    adapter->logical_device.destroy(adapter->font_tex);
    adapter->logical_device.destroy(adapter->font_image_view);

    adapter->logical_device.destroy(adapter->pipeline_layout);
    adapter->logical_device.destroy(adapter->render_pass);
    adapter->logical_device.destroy(adapter->pipeline);

    adapter->logical_device.destroy(adapter->descriptor_set_layout);
    adapter->logical_device.destroy(adapter->descriptor_pool);
    adapter->logical_device.destroy(adapter->command_pool);

    nk_font_atlas_clear(&glfw.atlas);
    nk_free(&glfw.ctx);
    nk_glfw3_device_destroy();
    memset(&glfw, 0, sizeof(glfw));
}

#endif

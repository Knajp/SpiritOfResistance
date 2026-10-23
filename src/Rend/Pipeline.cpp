#include "Renderer.hpp"
#include "../Logger.hpp"
#include <vulkan/vulkan_core.h>

namespace game
{
VkPipeline Renderer::createGraphicsPipeline(VkPipelineLayout& layout, VkShaderModule vertexShaderModule, VkShaderModule fragmentShaderModule)
{
  VkPipelineLayoutCreateInfo layoutInfo
  {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 0,
    .pushConstantRangeCount = 0
  };

  if(vkCreatePipelineLayout(mDevice, &layoutInfo, nullptr, &layout) != VK_SUCCESS)
  {
    spdlog::error("Failed to create graphics pipeline layout!");
    throw std::runtime_error("err");
  }

  VkPipelineShaderStageCreateInfo vertexStage
  {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
    .module = vertexShaderModule,
    .pName = "main"
  };

  VkPipelineShaderStageCreateInfo fragmentStage
  {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    .module = fragmentShaderModule,
    .pName = "main"
  };

  VkPipelineShaderStageCreateInfo stages[] = {vertexStage, fragmentStage};

  VkPipelineColorBlendAttachmentState colorAtt{
    .blendEnable = VK_FALSE,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
  };
  VkPipelineColorBlendStateCreateInfo colorBlend{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments = &colorAtt,
    .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
  };

  VkPipelineMultisampleStateCreateInfo multisample{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
  };

  VkPipelineRasterizationStateCreateInfo rasterizer{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .lineWidth = 1.0f,
    .depthBiasEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
  };

  VkPipelineVertexInputStateCreateInfo vertexInput{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 0,
    .vertexAttributeDescriptionCount = 0,
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_TRUE,
  };
  
  VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = 2,
    .pDynamicStates = dynamicStates 
  };

  VkGraphicsPipelineCreateInfo createInfo {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .stageCount = 2,
    .pStages = stages,
    .pVertexInputState = &vertexInput,
    .pInputAssemblyState = &inputAssembly,
    .pRasterizationState = &rasterizer,
    .pMultisampleState = &multisample,
    .pColorBlendState = &colorBlend,
    .pDynamicState = &dynamicState,
    .layout = layout,
    
  };

  VkPipeline graphicsPipeline;
  if(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &createInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
  {
    spdlog::error("Failed to create graphics pipeline!");
    throw std::runtime_error("err");
  }

  spdlog::info("Created graphics pipeline.");

  return graphicsPipeline;

}
}

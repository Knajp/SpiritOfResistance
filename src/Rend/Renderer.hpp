#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <GLFW/glfw3.h>
#include <volk/volk.h>
#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>
#include <vector>

#include <optional>

#include "Shader.hpp"

namespace game::RenderUtil
{
struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  std::optional<uint32_t> transferFamily;
  std::optional<uint32_t> computeFamily;

  bool isComplete() const
  {
    return graphicsFamily.has_value() && transferFamily.has_value() && computeFamily.has_value() && presentFamily.has_value();
  }
};

struct SwapchainSupportDetails
{
  std::vector<VkSurfaceFormat2KHR> surfaceFormats;
  std::vector<VkPresentModeKHR> presentModes;
  VkSurfaceCapabilities2KHR capabilities;
};
}
namespace game
{
class Renderer
{
public:
  static Renderer& getInstance()
  {
    static Renderer instance;
    return instance;
  }

  void initRenderer(GLFWwindow* window)
  {
    createVulkanInstance();
    createWindowSurface(window);
    createDebugMessenger(); 
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain(window);
    mGraphicsPipeline = createGraphicsPipeline(mGraphicsPipelineLayout, shader::ShaderTool::createShaderModule("src/shader/shader.vert", EShLangVertex, mDevice), shader::ShaderTool::createShaderModule("/src/shader/shader.frag", EShLangFragment, mDevice));
  }
private:
  void createVulkanInstance();
  void initializeVMA();
  void createDebugMessenger();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createWindowSurface(GLFWwindow* window);
  void createSwapchain(GLFWwindow* window);

  RenderUtil::QueueFamilyIndices findFamilyIndices(VkPhysicalDevice device);
  RenderUtil::SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device) const;

  VkPipeline createGraphicsPipeline(VkPipelineLayout& layout, VkShaderModule vertexShaderModule, VkShaderModule fragmentShaderModule);

  VkInstance mInstance = VK_NULL_HANDLE;
  VmaAllocator mVmaAllocator;
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
  VkDevice mDevice = VK_NULL_HANDLE;

  VkSurfaceKHR mWindowSurface;
  
  VkSwapchainKHR mSwapchain;
  VkPresentModeKHR mPresentMode;
  VkSurfaceFormat2KHR mSwapchainFormat;
  VkExtent2D mSwapchainExtent;
  VkPresentModeKHR mSwapchainPresentMode;
  VkPipeline mGraphicsPipeline;
  VkPipelineLayout mGraphicsPipelineLayout;

  VkQueue mGraphicsQueue = VK_NULL_HANDLE;
  VkQueue mTransferQueue = VK_NULL_HANDLE;
  VkQueue mComputeQueue = VK_NULL_HANDLE;
  //VkQueue mPresentQueue = VK_NULL_HANDLE;
};

}
#endif 

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <GLFW/glfw3.h>
#include <volk/volk.h>
#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>
#include <optional>

namespace game::RenderUtil
{
struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
 // std::optional<uint32_t> presentFamily;
  std::optional<uint32_t> transferFamily;
  std::optional<uint32_t> computeFamily;

  bool isComplete() const
  {
    return graphicsFamily.has_value() && transferFamily.has_value() && computeFamily.has_value();
  }
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

  void initRenderer()
  {
    createVulkanInstance();
    createDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
  }
private:
  void createVulkanInstance();
  void initializeVMA();
  void createDebugMessenger();
  void pickPhysicalDevice();
  void createLogicalDevice();
  RenderUtil::QueueFamilyIndices findFamilyIndices(VkPhysicalDevice device);

  VkInstance mInstance = VK_NULL_HANDLE;
  VmaAllocator mVmaAllocator;
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
  VkDevice mDevice = VK_NULL_HANDLE;

  VkQueue mGraphicsQueue = VK_NULL_HANDLE;
  VkQueue mTransferQueue = VK_NULL_HANDLE;
  VkQueue mComputeQueue = VK_NULL_HANDLE;
  //VkQueue mPresentQueue = VK_NULL_HANDLE;
};

}
#endif 

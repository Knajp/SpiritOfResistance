#include "Renderer.hpp"
#include <vector>
#include <iostream>
#include <map>
#include <cassert>
#include <set>
#include <vulkan/vulkan_core.h>
#include "../Logger.hpp"

namespace game
{
static std::vector<const char*> instanceExtensions{
  #ifdef DEBUG
  VK_EXT_DEBUG_UTILS_EXTENSION_NAME
  #endif
};
static std::vector<const char*> instanceLayers{
  #ifdef DEBUG
   "VK_LAYER_KHRONOS_validation"
  #endif
};
static std::vector<const char*> deviceLayers{};
static std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                    VkDebugUtilsMessageTypeFlagsEXT type,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* userData)
{
  std::cerr << "[Vulkan]" << pCallbackData->pMessage << "\n";
  return VK_FALSE;
}

void Renderer::createDebugMessenger()
{
  VkDebugUtilsMessengerCreateInfoEXT createInfo
  {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = debugCallback 
      
  };

  if(vkCreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS)
  {
    spdlog::error("Failed to create Debug Utils Messenger.");
    throw std::runtime_error("err");
  }

  spdlog::info("Created Debug Utils Messenger");
}
void Renderer::createVulkanInstance()
{
  volkInitialize();

  VkApplicationInfo appInfo{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,  
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "None",
    .engineVersion = VK_MAKE_VERSION(1,0,0),
    .apiVersion = VK_API_VERSION_1_4,
  };

  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  if(!glfwExtensions)
  {
    spdlog::error("Failed to get required GLFW instance extensions");
    throw std::runtime_error("err"); 
  }
  for(uint32_t i = 0; i < glfwExtensionCount; i++)
      instanceExtensions.push_back(glfwExtensions[i]);


  VkInstanceCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0, 
    .pApplicationInfo = &appInfo,
    .enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
    .ppEnabledLayerNames = instanceLayers.data(),  
    .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
    .ppEnabledExtensionNames = instanceExtensions.data(),
  };

  if(vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
  {
    spdlog::critical("FAILED TO CREATE VULKAN INSTANCE!");
    throw std::runtime_error("crit-err");
  }

  spdlog::info("Created Vulkan instance");

  volkLoadInstance(mInstance);

  spdlog::info("Volk loaded instance");
}

void Renderer::initializeVMA()
{
  VmaVulkanFunctions vmaFuncInfo{};
  VmaAllocatorCreateInfo vmaAllocInfo{
    .physicalDevice = mPhysicalDevice,
    .device = mDevice,
    .pVulkanFunctions = &vmaFuncInfo,
    .instance = mInstance,
    .vulkanApiVersion = VK_API_VERSION_1_4
  };

  vmaImportVulkanFunctionsFromVolk(&vmaAllocInfo, &vmaFuncInfo);

  if(vmaCreateAllocator(&vmaAllocInfo, &mVmaAllocator) != VK_SUCCESS)
  {
    spdlog::critical("FAILED TO CREATE VMA ALLOCATOR!");
    throw std::runtime_error("crit-err");
  }
  spdlog::info("Created VMA allocator");
}
bool checkDeviceExtensionSupport(VkPhysicalDevice device)
{
  uint32_t extensionCount = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for(const auto& extension : extensions)
  {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}
void Renderer::pickPhysicalDevice()
{
  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr);
  std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
  vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, physicalDevices.data());

  assert(physicalDeviceCount > 0);
  assert(physicalDevices.size() > 0);

  std::multimap<int, VkPhysicalDevice> candidates{};

  for(const auto& device : physicalDevices)
  {
    int score = 0;

    VkPhysicalDeviceFeatures2 features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    vkGetPhysicalDeviceFeatures2(device, &features);
    VkPhysicalDeviceProperties2 properties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    vkGetPhysicalDeviceProperties2(device, &properties);

    if(!features.features.geometryShader)
      continue;

    if(!checkDeviceExtensionSupport(device))
      continue;

    RenderUtil::SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);
    if(swapchainSupport.presentModes.empty() || swapchainSupport.surfaceFormats.empty()) continue;

    constexpr uint16_t discreteBoost = 1000;
    if(properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += discreteBoost;

    candidates.insert(std::make_pair(score,device));
  }

  assert(candidates.size() >0);

  if(candidates.rbegin()->first > 0)
    mPhysicalDevice = candidates.rbegin()->second;
  else {spdlog::error("No suitable GPU found on client!"); throw std::runtime_error("err");}
}

RenderUtil::QueueFamilyIndices Renderer::findFamilyIndices(VkPhysicalDevice device) {
  RenderUtil::QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties2> queueFamilyProperties(queueFamilyCount);
  for(auto& queueFamily : queueFamilyProperties)
    queueFamily.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;

  vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueFamilyCount, queueFamilyProperties.data());

  int i = 0;
  for(const auto& qfp : queueFamilyProperties)
  {
    if(qfp.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      indices.graphicsFamily = i;
    if(qfp.queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
      indices.computeFamily = i;
    if(qfp.queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT)
      indices.transferFamily = i;

    VkBool32 presentSupport = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mWindowSurface, &presentSupport);
    if(presentSupport)
      indices.presentFamily = i;

    i++;
  }

  return indices;

}
void Renderer::createLogicalDevice()
{
  VkPhysicalDeviceVulkan14Features supportedFeatures14{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES, .pNext = nullptr};
  VkPhysicalDeviceVulkan13Features supportedFeatures13{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &supportedFeatures14};
  VkPhysicalDeviceVulkan12Features supportedFeatures12{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext  = &supportedFeatures13 };
  VkPhysicalDeviceFeatures2 supportedFeatures11{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &supportedFeatures12 };
  vkGetPhysicalDeviceFeatures2(mPhysicalDevice, &supportedFeatures11);

  VkPhysicalDeviceProperties2 props{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
  vkGetPhysicalDeviceProperties2(mPhysicalDevice, &props);

  spdlog::debug(props.properties.deviceName);
  if(!supportedFeatures13.dynamicRendering || !supportedFeatures13.synchronization2 || !supportedFeatures12.timelineSemaphore)
  {
    spdlog::error("Physical device doesn't fulfill application Vulkan 1.X feature requirements.");
    throw std::runtime_error("err");
  }
  VkPhysicalDeviceVulkan14Features features14{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES, .pNext = nullptr};
  VkPhysicalDeviceVulkan13Features features13{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &features14, .synchronization2 = VK_TRUE, .dynamicRendering = VK_TRUE};
  VkPhysicalDeviceVulkan12Features features12{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &features13, .timelineSemaphore = VK_TRUE};
  VkPhysicalDeviceFeatures2 features11{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &features12};

  RenderUtil::QueueFamilyIndices indices = findFamilyIndices(mPhysicalDevice);
  float queuePriorities = 1.0f;

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueIndices = {indices.graphicsFamily.value(), indices.computeFamily.value(), indices.transferFamily.value()};

  for(const auto& idc : uniqueQueueIndices)
  {
    VkDeviceQueueCreateInfo queueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = idc,
      .queueCount = 1,
      .pQueuePriorities = &queuePriorities,
    };

    queueCreateInfos.push_back(queueCreateInfo);
  }
  VkDeviceCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = &features11,
    .flags = 0,
    .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
    .pQueueCreateInfos = queueCreateInfos.data(),
    .enabledLayerCount = static_cast<uint32_t>(deviceLayers.size()),
    .ppEnabledLayerNames = deviceLayers.data(),
    .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
    .ppEnabledExtensionNames = deviceExtensions.data() 
  };

  if(vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS)
  {
    spdlog::critical("FAILED TO CREATE LOGICAL DEVICE!");
    throw std::runtime_error("crit-err");
  }
  vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
  vkGetDeviceQueue(mDevice, indices.transferFamily.value(), 0, &mTransferQueue);
  vkGetDeviceQueue(mDevice, indices.computeFamily.value(), 0, &mComputeQueue);
  //vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);

  spdlog::info("Created logical device and acquired queues.");
}

void Renderer::createWindowSurface(GLFWwindow* window)
{
  if(glfwCreateWindowSurface(mInstance, window, nullptr, &mWindowSurface) != VK_SUCCESS)
  {
    spdlog::error("Failed to create window surface.");
    throw std::runtime_error("err");
  }
  spdlog::info("Created window surface.");
}

RenderUtil::SwapchainSupportDetails Renderer::querySwapchainSupport(VkPhysicalDevice device) const
{
  RenderUtil::SwapchainSupportDetails swapchainSupport{};

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, mWindowSurface, &presentModeCount, nullptr);
  swapchainSupport.presentModes.resize(presentModeCount);
  if(presentModeCount > 0)
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, mWindowSurface, &presentModeCount, swapchainSupport.presentModes.data());

  VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
    .surface = mWindowSurface 
  };

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surfaceInfo, &formatCount, nullptr);
  swapchainSupport.surfaceFormats.resize(formatCount);
  if(formatCount > 0)
    vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surfaceInfo, &formatCount, swapchainSupport.surfaceFormats.data());

  vkGetPhysicalDeviceSurfaceCapabilities2KHR(device, &surfaceInfo, &swapchainSupport.capabilities);

  return swapchainSupport;
}

void Renderer::createSwapchain(GLFWwindow* window)
{
  RenderUtil::SwapchainSupportDetails supportDetails = querySwapchainSupport(mPhysicalDevice);

  mSwapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
  for(const auto& mode : supportDetails.presentModes)
  {
    if(mode != VK_PRESENT_MODE_MAILBOX_KHR)
      continue;

    mSwapchainPresentMode = mode;
  }

  mSwapchainFormat = supportDetails.surfaceFormats[0];
  for(const auto& format : supportDetails.surfaceFormats)
  {
    if(format.surfaceFormat.format != VK_FORMAT_B8G8R8A8_SRGB || format.surfaceFormat.colorSpace != VK_COLORSPACE_SRGB_NONLINEAR_KHR)
      continue;

    mSwapchainFormat = format;
  }

  if(supportDetails.capabilities.surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    mSwapchainExtent = supportDetails.capabilities.surfaceCapabilities.currentExtent;
  else
  {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
  
    actualExtent.width = std::clamp(actualExtent.width, supportDetails.capabilities.surfaceCapabilities.minImageExtent.width, supportDetails.capabilities.surfaceCapabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, supportDetails.capabilities.surfaceCapabilities.minImageExtent.height, supportDetails.capabilities.surfaceCapabilities.maxImageExtent.height);

    mSwapchainExtent = actualExtent;
  }

  uint32_t imageCount = supportDetails.capabilities.surfaceCapabilities.minImageCount + 1;
  if(supportDetails.capabilities.surfaceCapabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.surfaceCapabilities.maxImageCount)
    imageCount = supportDetails.capabilities.surfaceCapabilities.maxImageCount;

  RenderUtil::QueueFamilyIndices indices = findFamilyIndices(mPhysicalDevice);
  uint32_t qfi[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  bool sameIndex = indices.presentFamily.value() == indices.graphicsFamily.value();

  VkSwapchainCreateInfoKHR createInfo{
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = mWindowSurface, 
    .minImageCount = imageCount,
    .imageFormat = mSwapchainFormat.surfaceFormat.format,
    .imageColorSpace = mSwapchainFormat.surfaceFormat.colorSpace,
    .imageExtent = mSwapchainExtent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = sameIndex ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
    .queueFamilyIndexCount = sameIndex ? (uint32_t)0 : (uint32_t)2,
    .pQueueFamilyIndices = sameIndex ? nullptr : qfi,
    .preTransform = supportDetails.capabilities.surfaceCapabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = mSwapchainPresentMode,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE,
  };
  
  if(vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapchain) != VK_SUCCESS)
  {
    spdlog::critical("Failed to create swap chain!");
    throw std::runtime_error("crit-err");
  }

  spdlog::info("Created swap chain.");
}
}

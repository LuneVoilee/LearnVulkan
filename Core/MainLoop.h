#pragma once

#include <algorithm>
#include <vector>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

class HelloTriangleApplication
{
public:
    HelloTriangleApplication() = default;

    void run();

private:
    void InitWindow();


    void InitVulkan();

    void MainLoop();

    void CleanUp();

    void                     CreateInstance();
    void                     HandleAppInfo(VkApplicationInfo& appInfo);
    void                     HandleCreateInfo(const VkApplicationInfo& appInfo , VkInstanceCreateInfo& createInfo);
    void                     GetExtensionInfo();
    bool                     CheckValidationLayerSupport();
    std::vector<const char*> GetRequiredExtensions();

    void            CreateDebugMessenger();
    void            HandleCreateInfo_DebugMessager(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VkBool32 DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity ,
                                  VkDebugUtilsMessageTypeFlagsEXT             messageType ,
                                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData , void* pUserData);


    void CreateSurface();


    void ChoosePhysicalDevice();
    void ChooseBestDevice(std::vector<VkPhysicalDevice> devices);
    int  CalculateScore(VkPhysicalDevice device);

    bool                    CheckPhysicsDevice(VkPhysicalDevice device);
    bool                    CheckDeviceExtensionSupport(VkPhysicalDevice device);
    int                     GetQueueFamiliesIndex(VkPhysicalDevice device , VkQueueFlagBits queueFlags);
    bool                    CheckQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails GetSwapChainDetails(VkPhysicalDevice device);
    bool                    CheckSwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR      ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR        ChooseSwapPresentMode(std::vector<VkPresentModeKHR> availableFormatsPresentModes);
    VkExtent2D              ChooseSwapResolution(const VkSurfaceCapabilitiesKHR& capabilities);

    void HandleCreateInfo_Device(VkDeviceQueueCreateInfo queueCreateInfo , VkPhysicalDeviceFeatures& deviceFeatures ,
                                 VkDeviceCreateInfo&     createInfo);
    VkSwapchainCreateInfoKHR HandleCreateInfo_SwapChain(
    );
    void CreateSwapChain();
    void HandleCreateInfo_DeviceQueue(VkDeviceQueueCreateInfo& queueCreateInfo , const float& queuePriority ,
                                      int                      queueFamilyIndex);

    void CreateLogicalDevice();


    //窗口相关
    GLFWwindow* m_Window;
    //Vulkan相关
    VkInstance               m_Instance;
    VkDebugUtilsMessengerEXT m_Messenger;
    //这一对象可以在VkInstance进行清除操作时，自动清除自己，所以我们不需要再cleanup函数中对它进行清除。
    VkSurfaceKHR         m_Surface;
    VkPhysicalDevice     m_PhysicalDevice;
    VkDevice             m_Device;
    VkQueue              m_GraphicsQueue;
    VkQueue              m_PresentQueue;
    VkSwapchainKHR       m_SwapChain;
    std::vector<VkImage> m_SwapChainImages;
    VkFormat             m_SwapChainImageFormat;
    VkExtent2D           m_SwapChainExtent;
};

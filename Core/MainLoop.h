#pragma once
#include <vector>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>


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

    void ChoosePhysicalDevice();
    void ChooseBestDevice(std::vector<VkPhysicalDevice> devices);
    int  CalculateScore(VkPhysicalDevice device);
    int  FindQueueFamilies(VkPhysicalDevice device , VkQueueFlagBits queueFlags);
    bool CheckQueueFamilies(VkPhysicalDevice device);
    void HandleCreateInfo_Device(VkDeviceQueueCreateInfo   queueCreateInfo ,
                                 VkPhysicalDeviceFeatures& deviceFeatures ,
                                 VkDeviceCreateInfo&       createInfo);
    void HandleCreateInfo_DeviceQueue(VkDeviceQueueCreateInfo& queueCreateInfo , const float& queuePriority ,
                                      int                      queueFamilyIndex);

    void                    CreateLogicalDevice();
    VkDeviceQueueCreateInfo HandleCreateInfo_DeviceQueue(VkDeviceQueueCreateInfo& createInfo ,
                                                         const float&             queuePriority);

    //窗口相关
    GLFWwindow* window;
    //Vulkan相关
    VkInstance               m_Instance;
    VkDebugUtilsMessengerEXT m_Messenger;
    //这一对象可以在VkInstance进行清除操作时，自动清除自己，所以我们不需要再cleanup函数中对它进行清除。
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice         m_Device;
    VkQueue          m_GraphicsQueue;
};

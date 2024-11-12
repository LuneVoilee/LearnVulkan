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
    void initWindow();

    
    void initVulkan();

    void mainLoop();

    void cleanup();

    void                     createInstance();
    void                     handleAppInfo(VkApplicationInfo& appInfo);
    void                     handleCreateInfo(const VkApplicationInfo& appInfo , VkInstanceCreateInfo& createInfo);
    void                     GetExtensionInfo();
    bool                     checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();

    void            createDebugMessenger();
    void            handleCreateInfo_DebugMessager(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity ,
                                  VkDebugUtilsMessageTypeFlagsEXT             messageType ,
                                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData , void* pUserData);

    void choosePhysicalDevice();
    void ChooseBestDevice(std::vector<VkPhysicalDevice> devices);
    int  CalculateScore(VkPhysicalDevice device);
    int  FindQueueFamilies(VkPhysicalDevice device , VkQueueFlagBits queueFlags);
    bool CheckQueueFamilies(VkPhysicalDevice device);

    void CreateLogicalDevice();
    
    //窗口相关
    GLFWwindow* window;
    //Vulkan相关
    VkInstance               m_Instance;
    VkDebugUtilsMessengerEXT m_Messenger;
    //这一对象可以在VkInstance进行清除操作时，自动清除自己，所以我们不需要再cleanup函数中对它进行清除。
    VkPhysicalDevice m_PhysicalDevice;
};

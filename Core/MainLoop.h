#pragma once
#include <vector>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

class HelloTriangleApplication
{
public:
    HelloTriangleApplication() = default;

    void run();
    /*
        int  GetWidth();
        void SetWidth();
        int  GetHeight();
        void SetHeight();
    */
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


    //窗口相关
    GLFWwindow* window;
    //Vulkan相关
    VkInstance               m_Instance;
    VkDebugUtilsMessengerEXT m_Messenger;
};

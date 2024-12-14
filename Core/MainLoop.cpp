#define GLFW_INCLUDE_VULKAN
#include "MainLoop.h"
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

#include "../Math/Math.h"


constexpr uint32_t Width  = 800;
constexpr uint32_t Height = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif
/*
 *填写完结构体信息后，我们将它作为一个参数调用vkCreateDebugUtilsMessengerEXT函数来创建VkDebugUtilsMessengerEXT对象。
 *由于vkCreateDebugUtilsMessengerEXT函数是一个扩展函数，不会被Vulkan库自动加载，所以需要我们自己使用vkGetInstanceProcAddr函数来加载它。
 *在这里，我们创建了一个代理函数，来载入vkCreateDebugUtilsMessengerEXT函数
 */
VkResult CreateDebugUtilsMessengerEXT(VkInstance                                instance ,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo ,
                                      const VkAllocationCallbacks*              pAllocator ,
                                      VkDebugUtilsMessengerEXT*                 pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance                   instance ,
                                   VkDebugUtilsMessengerEXT     debugMessenger ,
                                   const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}


void HelloTriangleApplication::run()
{
    InitWindow();
    InitVulkan();
    MainLoop();
    CleanUp();
}


void HelloTriangleApplication::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    //glfwCreateWindow函数的前三个参数指定了要创建的窗口的宽度，高度和标题。
    //第四个参数用于指定在哪个显示器上打开窗口，最后一个参数与OpenGL相关，对我们没有意义。
    m_Window = glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
    if (m_Window == nullptr)
    {
        throw std::runtime_error("创建窗口失败");
    }
}

void HelloTriangleApplication::InitVulkan()
{
    CreateInstance();
    CreateDebugMessenger();
    CreateSurface();
    ChoosePhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
}

void HelloTriangleApplication::MainLoop()
{
    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();
    }
}

void HelloTriangleApplication::CleanUp()
{
    vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
    vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);

    for (auto imageView : m_ImageViews)
    {
        vkDestroyImageView(m_Device, imageView, nullptr);
    }

    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(m_Instance, m_Messenger, nullptr);
    }

    vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
    vkDestroyDevice(m_Device, nullptr);
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void HelloTriangleApplication::CreateInstance()
{
    if (enableValidationLayers && !CheckValidationLayerSupport())
    {
        throw std::runtime_error("使用了不被支持的校验层");
    }
    //应用程序信息
    //这个结构体不是必须的，但是它可以帮助驱动程序优化应用程序。
    //比如，应用程序使用了某个引擎，驱动程序对这个引擎有一些特殊处理，这时就可能有很大的优化提升
    VkApplicationInfo appInfo = {};
    HandleAppInfo(appInfo);

    //创建信息
    //这个结构体是创建一个Vulkan实例时必须填写的信息。
    //它告诉Vulkan的驱动程序需要使用的全局扩展和校验层。全局是指这里的设置对于整个应用程序都有效，而不仅仅对一个设备有效。
    VkInstanceCreateInfo createInfo = {};
    HandleCreateInfo(appInfo, createInfo);

    //创建实例
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("创建实例失败");
    }
}

void HelloTriangleApplication::HandleAppInfo(VkApplicationInfo& appInfo)
{
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;
}

void HelloTriangleApplication::HandleCreateInfo(const VkApplicationInfo& appInfo , VkInstanceCreateInfo& createInfo)
{
    createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    //将所需的拓展封装进getRequiredExtensions
    auto glfwExtensions                = GetRequiredExtensions();
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(glfwExtensions.size());
    createInfo.ppEnabledExtensionNames = glfwExtensions.data();

    //校验层信息
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        HandleCreateInfo_DebugMessager(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }
}


void HelloTriangleApplication::GetExtensionInfo()
{
    //首先获取VK支持的扩展的数量
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    //获取数量后便可以通过数组获取所有扩展信息：
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    //每个VkExtensionProperties结构体包含了扩展的名字和版本信息。我们可以使用下面的代码将这些信息打印在控制台窗口中
    std::cout << "available extensions:" << '\n';
    for (const auto& extension : extensions)
    {
        std::cout << "\t" << extension.extensionName << '\n';
    }
}

bool HelloTriangleApplication::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }
    return true;
}

std::vector<const char*> HelloTriangleApplication::GetRequiredExtensions()
{
    //Vulkan是平台无关的API，所以需要一个和窗口系统交互的扩展。
    //我们通过GLFW库里的glfwGetRequiredInstanceExtensions返回Vulkan所需的扩展。
    uint32_t     glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions,
                                        glfwExtensions + glfwExtensionCount);

    //启用校验层所需的拓展
    if (enableValidationLayers)
    {
        //VK_EXT_DEBUG_UTILS_EXTENSION_NAME 等价于 VK_EXT_debug_utils
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void HelloTriangleApplication::HandleCreateInfo_DebugMessager(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    createInfo.pUserData       = nullptr;
}

void HelloTriangleApplication::CreateDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    HandleCreateInfo_DebugMessager(createInfo);

    CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_Messenger);
}

/*
 * 函数的第一个参数指定了消息的级别，它有四种级别：
 *   VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT：诊断信息
 *   VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT：资源创建之类的信息
 *   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT：警告信息
 *   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT：不合法和可能造成崩溃的操作信息
 *
 *   这些值经过一定设计，可以使用比较运算符来过滤处理一定级别以上的调试信息：
 *   if (messageSeverity >=VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
 *   {
 *       // Message is important enough to show
 *   }
 */

/*
 *   messageType参数可以是下面这些值：
 *
 *   VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT：发生了一些与规范和性能无关的事件
 *
 *   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT：出现了违反规范的情况或发生了一个可能的错误
 *
 *   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT：进行了可能影响Vulkan性能的行为
 *
 * 
 */

/*
 * pCallbackData参数是一个指向VkDebugUtilsMessengerCallbackDataEXT结构体的指针，
 * 这一结构体包含了下面这些非常重要的成员：
 *
 * pMessage：一个以null结尾的包含调试信息的字符串
 *
 * pObjects：存储有和消息相关的Vulkan对象句柄的数组
 *
 * objectCount：数组中的对象个数
 *
 */

/*
 * 最后一个参数pUserData是一个指向用户自定义数据的指针，它是可选的，
 * 这个指针所指的地址会被作为回调函数的参数，用来向回调函数传递用户数据。
 */
VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::DebugCallback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity ,
    VkDebugUtilsMessageTypeFlagsEXT             messageType ,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData ,
    void*                                       pUserData
)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << '\n';
    return VK_FALSE;
}

void HelloTriangleApplication::CreateSurface()
{
    if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

void HelloTriangleApplication::ChoosePhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    //步骤封装到ChooseBestDevice函数中
    ChooseBestDevice(devices);

    if (m_PhysicalDevice == nullptr)
    {
        throw std::runtime_error("没有可用的GPU");
    }
}

void HelloTriangleApplication::ChooseBestDevice(std::vector<VkPhysicalDevice> devices)
{
    //可以通过计算每张显卡的分数来选择最适合的显卡
    int maxScore = 0;
    for (auto device : devices)
    {
        if (!CheckPhysicsDevice(device))
            continue;

        int score = CalculateScore(device);
        if (score > maxScore)
        {
            maxScore         = score;
            m_PhysicalDevice = device;
        }
    }
}

int HelloTriangleApplication::CalculateScore(VkPhysicalDevice device)
{
    //详细的设备信息：名称，类型和支持的Vulkan版本等
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    //支持的特性：纹理压缩，64位浮点和多视口渲染(常用于VR)等
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    int score = 0;
    //离散GPU比集成GPU更好
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }

    //最好支持的纹理压缩格式是BC1-BC3，因为它们是所有硬件支持的格式
    if (deviceFeatures.textureCompressionBC)
    {
        score += 1000;
    }

    // 最大纹理大小影响图形的质量
    score += deviceProperties.limits.maxImageDimension2D;

    /*
     * 假设应用程序必须要求支持几何着色器 
    if (!deviceFeatures.geometryShader) {
        return 0;
    }
    */
    return score;
}

bool HelloTriangleApplication::CheckPhysicsDevice(VkPhysicalDevice device)
{
    return CheckQueueFamilies(device) &&
            CheckDeviceExtensionSupport(device) &&
            CheckSwapChainSupport(device);
}

int HelloTriangleApplication::GetQueueFamiliesIndex(VkPhysicalDevice device , VkQueueFlagBits queueFlags)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
        //既有图像能力，又有呈现支持
        if (queueFamily.queueCount > 0 && ( queueFamily.queueFlags & queueFlags ) && presentSupport)
        {
            return i;
        }
        i++;
    }
    return -1;
}


//检查设备有没有所需的队列族
bool HelloTriangleApplication::CheckQueueFamilies(VkPhysicalDevice device)
{
    int index = GetQueueFamiliesIndex(device, VK_QUEUE_GRAPHICS_BIT);
    return index != -1;
}

//检查设备有没有所需的拓展
bool HelloTriangleApplication::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> tempSet = {deviceExtensions.begin(), deviceExtensions.end()};
    for (const auto& extension : availableExtensions)
    {
        tempSet.erase(extension.extensionName);
    }

    return tempSet.empty();
}

SwapChainSupportDetails HelloTriangleApplication::GetSwapChainDetails(VkPhysicalDevice device)
{
    SwapChainSupportDetails details = {};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

//检测交换链与窗口表面是否兼容
bool HelloTriangleApplication::CheckSwapChainSupport(VkPhysicalDevice device)
{
    //不应使用std::move，不仅会导致代码冗余，而且会导致copy elision失效
    auto details = GetSwapChainDetails(device);
    return !details.formats.empty() && !details.presentModes.empty();
}

VkSurfaceFormatKHR HelloTriangleApplication::ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    //为VK_FORMAT_UNDEFINED表明表面没有自己的首选格式，这时，我们直接使用我们设定的格式
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace ==
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApplication::ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> availablePresentModes)
{
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& availablePresentMode : availablePresentModes)
    {
        // VK_PRESENT_MODE_MAILBOX_KHR三重缓冲，是最好的
        // > VK_PRESENT_MODE_IMMEDIATE_KHR立即显示 
        // > VK_PRESENT_MODE_FIFO_KHR类似垂直同步（许多驱动程序对VK_PRESENT_MODE_FIFO_KHR呈现模式的支持不够好）
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            bestMode = availablePresentMode;
            break;
        }
        if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR && bestMode != VK_PRESENT_MODE_MAILBOX_KHR)
        {
            bestMode = availablePresentMode;
        }
    }

    return bestMode;
}

VkExtent2D HelloTriangleApplication::ChooseSwapResolution(const VkSurfaceCapabilitiesKHR& capabilities)
{
    //Vulkan通过currentExtent成员变量来告知适合我们窗口的交换范围。
    //一些窗口系统会使用一个特殊值，uint32_t变量类型的最大值，表示允许我们自己选择对于窗口最合适的交换范围
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    VkExtent2D actualExtent;

    //关于我只在这个文件里使用std::clamp会出现识别不到的情况，花了一个小时无法解决，所以不得不自己大无语手写Clamp函数这件事😅
    actualExtent.width  = Math::Clamp(Width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = Math::Clamp(Height, capabilities.minImageExtent.height,
                                      capabilities.maxImageExtent.height);
    return actualExtent;
}

void HelloTriangleApplication::CreateLogicalDevice()
{
    //创建逻辑设备需要先创建队列
    VkDeviceQueueCreateInfo queueCreateInfo  = {};
    float                   queuePriority    = 1.0f;
    int                     queueFamilyIndex = GetQueueFamiliesIndex(m_PhysicalDevice, VK_QUEUE_GRAPHICS_BIT);
    HandleCreateInfo_DeviceQueue(queueCreateInfo, queuePriority, queueFamilyIndex);

    //设备特性
    VkPhysicalDeviceFeatures deviceFeatures = {};

    //创建逻辑设备
    VkDeviceCreateInfo createInfo = {};
    HandleCreateInfo_Device(queueCreateInfo, deviceFeatures, createInfo);

    if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }
    //两个指针指向同一个队列，因为它既支持图形又支持呈现
    vkGetDeviceQueue(m_Device, queueFamilyIndex, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, queueFamilyIndex, 0, &m_PresentQueue);
}

void HelloTriangleApplication::HandleCreateInfo_DeviceQueue(VkDeviceQueueCreateInfo& queueCreateInfo ,
                                                            const float& queuePriority , int queueFamilyIndex)
{
    queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
}

void HelloTriangleApplication::HandleCreateInfo_Device(VkDeviceQueueCreateInfo   queueCreateInfo ,
                                                       VkPhysicalDeviceFeatures& deviceFeatures ,
                                                       VkDeviceCreateInfo&       createInfo)
{
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos       = &queueCreateInfo;
    createInfo.queueCreateInfoCount    = 1;
    createInfo.pEnabledFeatures        = &deviceFeatures;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    //让设备和实例使用相同的校验层
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }
}

void HelloTriangleApplication::CreateSwapChain()
{
    VkSwapchainCreateInfoKHR createInfo = HandleCreateInfo_SwapChain();

    if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("创建交换链失败！");
    }

    vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &createInfo.minImageCount, nullptr);
    m_SwapChainImages.resize(createInfo.minImageCount);
    vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &createInfo.minImageCount, m_SwapChainImages.data());
}

VkSwapchainCreateInfoKHR HelloTriangleApplication::HandleCreateInfo_SwapChain()
{
    SwapChainSupportDetails swapChainDetails = GetSwapChainDetails(m_PhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainDetails.formats);
    VkPresentModeKHR   presentMode   = ChooseSwapPresentMode(swapChainDetails.presentModes);
    VkExtent2D         extent        = ChooseSwapResolution(swapChainDetails.capabilities);

    m_SwapChainImageFormat = surfaceFormat.format;
    m_SwapChainExtent      = extent;


    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface                  = m_Surface;

    //imageCount：能够支持的缓冲区的个数
    //使用minImageCount + 1来支持三倍缓冲
    uint32_t imageCount = swapChainDetails.capabilities.minImageCount + 1;
    //maxImageCount的值为0表明，可以使用任意数量的缓冲区。所以只有maxImageCount的值大于0的时候需要检查。
    if (swapChainDetails.capabilities.maxImageCount > 0 && imageCount > swapChainDetails.capabilities.maxImageCount)
    {
        imageCount = swapChainDetails.capabilities.maxImageCount;
    }
    createInfo.minImageCount = imageCount;

    createInfo.imageFormat     = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent     = extent;

    //mageArrayLayers成员变量用于指定每个图像所包含的层次。对于非立体3D应用,它的值为1。但对于VR等应用程序来说，会使用更多的层次。
    createInfo.imageArrayLayers = 1;

    //imageUsage成员变量用于指定我们将在图像上进行怎样的操作。我们在图像上进行绘制操作，也就是将图像作为一个颜色附着来使用。
    //如果读者需要对图像进行后期处理之类的操作，可以使用VK_IMAGE_USAGE_TRANSFER_DST_BIT作为imageUsage成员变量的值，让交换链图像可以作为传输的目的图像。
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    int index = GetQueueFamiliesIndex(m_PhysicalDevice, VK_QUEUE_GRAPHICS_BIT);


    //VK_SHARING_MODE_EXCLUSIVE：一张图像同一时间只能被一个队列族所拥有，在另一队列族使用它之前，必须显式地改变图像所有权。
    //这一模式下性能表现最佳。
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    /*
     * 如果不要求队列族必须支持两个功能：
     *     //VK_SHARING_MODE_CONCURRENT：图像可以在多个队列族间使用，不需要显式地改变图像所有权。
     *     //协同模式需要我们使用queueFamilyIndexCount和pQueueFamilyIndices来指定共享所有权的队列族。
     *     //如果图形队列族和呈现队列族是同一个队列族(大部分情况下都是这样)，我们就不能使用协同模式，协同模式需要我们指定至少两个不同的队列族。
     *     createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
     *     //VK_SHARING_MODE_EXCLUSIVE模式没必要设置这两个
     *     createInfo.queueFamilyIndexCount = 2;
     *     createInfo.pQueueFamilyIndices = {index1 , index2};
     */

    //我们可以为交换链中的图像指定一个固定的变换操作(需要交换链具有supportedTransforms特性)，比如顺时针旋转90度或是水平翻转。
    //如果不需要进行任何变换操作，指定使用currentTransform变换即可。
    createInfo.preTransform = swapChainDetails.capabilities.currentTransform;

    //compositeAlpha成员变量用于指定alpha通道是否被用来和窗口系统中的其它窗口进行混合操作。
    //通常，我们将其设置为VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR来忽略掉alpha通道。
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    //presentMode成员变量用于设置呈现模式。clipped成员变量被设置为VK_TRUE,表示我们不关心被窗口系统中的其它窗口遮挡的像素的颜色，这允许Vulkan采取一定的优化措施，但如果我们回读窗口的像素值就可能出现问题。
    createInfo.presentMode = presentMode;
    createInfo.clipped     = VK_TRUE;

    //最后是oldSwapchain成员变量，需要指定它，是因为应用程序在运行过程中交换链可能会失效。比如，改变窗口大小后，交换链需要重建，重建时需要之前的交换链
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    return createInfo;
}

void HelloTriangleApplication::CreateImageViews()
{
    m_ImageViews.resize(m_SwapChainImages.size());
    for (size_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image                 = m_SwapChainImages[i];
        createInfo.viewType              = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format                = m_SwapChainImageFormat;

        //默认映射：红->红,绿->绿,蓝->蓝,透明->透明。可以把输入的值改到任意通道上 或 0/1
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT; // 操作颜色数据
        createInfo.subresourceRange.baseMipLevel   = 0; // 从最高分辨率开始
        createInfo.subresourceRange.levelCount     = 1; // 只操作第 0 层 mipmap
        createInfo.subresourceRange.baseArrayLayer = 0; // 从第 0 层数组开始（Vulkan 支持数组纹理，图像可以包含多个层，每层代表一个 2D 图像）
        createInfo.subresourceRange.layerCount     = 1; // 只操作第 0 层数组
    }
}

void HelloTriangleApplication::CreateRenderPass()
{
    //只使用一个代表交换链图像的颜色缓冲附着
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format                  = m_SwapChainImageFormat;
    colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;

    //loadOp和storeOp成员变量用于指定在渲染之前和渲染之后对附着中的数据进行的操作
    colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    //loadOp和storeOp成员变量的设置会对颜色和深度缓冲起效。
    //stencilLoadOp成员变量和stencilStoreOp成员变量会对模板缓冲起效
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    //图像布局方式与这个图像的使用目的相关
    //initialLayout渲染流程开始前的图像布局方式。finalLayout渲染流程结束后的图像布局方式.
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    //一个渲染流程可以包含多个子流程。子流程依赖于上一流程处理后的帧缓冲内容。
    //比如，许多叠加的后期处理效果就是在上一次的处理结果上进行的。
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment            = 0;
    //Vulkan会在子流程开始时自动将引用的附着转换到layout成员变量指定的图像布局。
    //我们推荐将layout成员变量设置为VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL，一般而言，它的性能表现最佳。
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //子流程的定义，刚才是子流程的附着
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    //这里设置的颜色附着在数组中的索引会被片段着色器使用，对应我们在片段着色器中使用的 layout(location = 0) out vec4 outColor
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount        = 1;
    renderPassInfo.pAttachments           = &colorAttachment;
    renderPassInfo.subpassCount           = 1;
    renderPassInfo.pSubpasses             = &subpass;

    if (vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void HelloTriangleApplication::CreateGraphicsPipeline()
{
    auto VertexShaderCode   = Loader::ReadFile("../Shaders/vert.spv");
    auto FragmentShaderCode = Loader::ReadFile("../Shaders/frag.spv");

    auto VertexShaderModule   = CreateShaderModule(VertexShaderCode);
    auto FragmentShaderModule = CreateShaderModule(FragmentShaderCode);

    vkDestroyShaderModule(m_Device, FragmentShaderModule, nullptr);
    vkDestroyShaderModule(m_Device, VertexShaderModule, nullptr);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage                           = VK_SHADER_STAGE_VERTEX_BIT; //指明用于哪个阶段
    vertShaderStageInfo.module                          = VertexShaderModule;
    vertShaderStageInfo.pName                           = "main"; //指明使用shader文件里的哪个函数。可以在一个文件里写多个着色器，通过不同的pName调用他们
    vertShaderStageInfo.pSpecializationInfo             = nullptr;
    /*
    *VkPipelineShaderStageCreateInfo还有一个可选的成员变量pSpecializationInfo
    *在这里，我们没有使用它，但这一成员变量非常值得我们在这里对它进行说明
    *我们可以通过这一成员变量指定着色器用到的常量
    *我们可以对同一个着色器模块对象指定不同的着色器常量用于管线创
    *这使得编译器可以根据指定的着色器常量来消除一些条件分支，这比在渲染时，使用变量配置着色器带来的效率要高得多。
    *如果不使用着色器常量，可以将pSpecializationInfo成员变量设置为nullptr。
    */

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage                           = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module                          = FragmentShaderModule;
    fragShaderStageInfo.pName                           = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    //描述传递给顶点着色器的顶点数据格式
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType                                = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount        = 0;
    vertexInputInfo.pVertexBindingDescriptions           = nullptr; //绑定：数据之间的间距和数据是按逐顶点的方式还是按逐实例的方式进行组织
    vertexInputInfo.vertexAttributeDescriptionCount      = 0;
    vertexInputInfo.pVertexAttributeDescriptions         = nullptr; //属性描述：传递给顶点着色器的属性类型，用于将属性绑定到顶点着色器中的变量

    //描述图元装配模式(topology) 和 是否启用几何图元重启
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology                               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable                 = VK_FALSE;

    //描述视口和裁剪矩形
    VkViewport viewport                             = {};
    viewport.x                                      = 0.0f;
    viewport.y                                      = 0.0f;
    viewport.width                                  = static_cast<float>(m_SwapChainExtent.width);
    viewport.height                                 = static_cast<float>(m_SwapChainExtent.height);
    viewport.minDepth                               = 0.0f;
    viewport.maxDepth                               = 1.0f;
    VkRect2D scissor                                = {};
    scissor.offset                                  = {0, 0};
    scissor.extent                                  = m_SwapChainExtent;
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount                     = 1;
    viewportState.pViewports                        = &viewport;
    viewportState.scissorCount                      = 1;
    viewportState.pScissors                         = &scissor;

    //描述光栅化方式
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable                       = VK_FALSE;
    //depthClampEnable成员变量设置为VK_TRUE表示在近平面和远平面外的片段会被截断为在近平面和远平面上，而不是直接丢弃这些片段。这对于阴影贴图的生成很有用。使用这一设置需要开启相应的GPU特性。
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    //rasterizerDiscardEnable成员变量设置为VK_TRUE表示所有几何图元都不能通过光栅化阶段。这一设置会禁止一切片段输出到帧缓冲。
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    //polygonMode成员变量用于指定多边形的填充模式。可以设置为VK_POLYGON_MODE_FILL填充模式，VK_POLYGON_MODE_LINE线框模式，VK_POLYGON_MODE_POINT点模式。
    rasterizer.lineWidth = 1.0f; //lineWidth成员变量用于指定光栅化后的线段宽度。线宽的最大值依赖于硬件，如果线宽度大于1.0f，需要开启相应的GPU特性。
    rasterizer.cullMode  = VK_CULL_MODE_BACK_BIT;
    //cullMode成员变量用于指定剔除模式。可以设置为VK_CULL_MODE_NONE不剔除任何图元，VK_CULL_MODE_FRONT_BIT剔除正面图元，VK_CULL_MODE_BACK_BIT剔除背面图元，VK_CULL_MODE_FRONT_AND_BACK剔除所有图元。
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    //frontFace成员变量用于指定多边形的正面是顺时针还是逆时针。可以设置为VK_FRONT_FACE_CLOCKWISE顺时针，VK_FRONT_FACE_COUNTER_CLOCKWISE逆时针。
    rasterizer.depthBiasEnable = VK_FALSE;
    //depthBiasEnable成员变量用于指定是否开启深度偏移。光栅化程序可以添加一个常量值或是一个基于片段所处线段的斜率得到的变量值到深度值上。这对于阴影贴图会很有用

    //多重采样技术 用于反走样，这里暂时禁用。
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType                                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable                  = VK_FALSE;
    multisampling.rasterizationSamples                 = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading                     = 1.0f;     // Optional
    multisampling.pSampleMask                          = nullptr;  // Optional
    multisampling.alphaToCoverageEnable                = VK_FALSE; // Optional
    multisampling.alphaToOneEnable                     = VK_FALSE; // Optional

    //颜色混合：有两个用于配置颜色混合的结构体。第一个是VkPipelineColorBlendAttachmentState结构体，可以用它来对每个绑定的帧缓冲进行单独的颜色混合配置。
    //第二个是VkPipelineColorBlendStateCreateInfo结构体，可以用它来进行全局的颜色混合配置。
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {}; //不配置
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;      // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;      // Optional
    /* 上方设置的作用
    if (blendEnable)
    {
        finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
        finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
    }
    else {
        finalColor = newColor;
    }
    finalColor = finalColor & colorWriteMask;
    */
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType                               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable                       = VK_FALSE;
    colorBlending.logicOp                             = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount                     = 1;
    colorBlending.pAttachments                        = &colorBlendAttachment;
    colorBlending.blendConstants[0]                   = 0.0f; // Optional
    colorBlending.blendConstants[1]                   = 0.0f; // Optional
    colorBlending.blendConstants[2]                   = 0.0f; // Optional
    colorBlending.blendConstants[3]                   = 0.0f; // Optional

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    //声明可以动态配置的内容
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType                            = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount                = 2;
    dynamicState.pDynamicStates                   = dynamicStates;

    //Uniform变量通过m_PipelineLayout在管线中提前定义
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount             = 0;       // Optional
    pipelineLayoutInfo.pSetLayouts                = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount     = 0;       // Optional
    pipelineLayoutInfo.pPushConstantRanges        = nullptr; // Optional

    if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount                   = 2;
    pipelineInfo.pStages                      = shaderStages;
    pipelineInfo.pVertexInputState            = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState          = &inputAssembly;
    pipelineInfo.pViewportState               = &viewportState;
    pipelineInfo.pRasterizationState          = &rasterizer;
    pipelineInfo.pMultisampleState            = &multisampling;
    pipelineInfo.pDepthStencilState           = nullptr; // Optional
    pipelineInfo.pColorBlendState             = &colorBlending;
    pipelineInfo.pDynamicState                = nullptr; // Optional

    pipelineInfo.layout     = m_PipelineLayout;
    pipelineInfo.renderPass = m_RenderPass;
    pipelineInfo.subpass    = 0;

    /*
    basePipelineHandle和basePipelineIndex成员变量用于以一个创建好的图形管线为基础创建一个新的图形管线。
    当要创建一个和已有管线大量设置相同的管线时，使用它的代价要比直接创建小，并且，对于从同一个管线衍生出的两个管线，在它们之间进行管线切换操作的效率也要高很多。
    我们可以使用basePipelineHandle来指定已经创建好的管线，或是使用basePipelineIndex来指定将要创建的管线作为基础管线，用于衍生新的管线。
    目前，我们只使用一个管线，所以将这两个成员变量分别设置为VK_NULL_HANDLE和-1，不使用基础管线衍生新的管线。
    这两个成员变量的设置只有在VkGraphicsPipelineCreateInfo结构体的flags成员变量使用了VK_PIPELINE_CREATE_DERIVATIVE_BIT标记的情况下才会起效。
    */
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex  = -1;             // Optional

    if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

VkShaderModule HelloTriangleApplication::CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize                 = code.size();
    createInfo.pCode                    = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }
    return shaderModule;
}

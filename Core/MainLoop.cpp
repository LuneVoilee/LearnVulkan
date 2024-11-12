#define GLFW_INCLUDE_VULKAN
#include "MainLoop.h"

#include <iostream>
#include <stdexcept>
#include <vector>
constexpr uint32_t Width = 800;
constexpr uint32_t Height = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
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
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}


void HelloTriangleApplication::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    //glfwCreateWindow函数的前三个参数指定了要创建的窗口的宽度，高度和标题。
    //第四个参数用于指定在哪个显示器上打开窗口，最后一个参数与OpenGL相关，对我们没有意义。
    window = glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
    if (window == nullptr)
    {
        throw std::runtime_error("创建窗口失败");
    }
}

void HelloTriangleApplication::initVulkan()
{
    createInstance();
    createDebugMessenger();
    choosePhysicalDevice();
    CreateLogicalDevice();
}

void HelloTriangleApplication::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void HelloTriangleApplication::cleanup()
{
    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(m_Instance, m_Messenger, nullptr);
    }

    vkDestroyInstance(m_Instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void HelloTriangleApplication::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("使用了不被支持的校验层");
    }
    //应用程序信息
    //这个结构体不是必须的，但是它可以帮助驱动程序优化应用程序。
    //比如，应用程序使用了某个引擎，驱动程序对这个引擎有一些特殊处理，这时就可能有很大的优化提升
    VkApplicationInfo appInfo = {};
    handleAppInfo(appInfo);

    //创建信息
    //这个结构体是创建一个Vulkan实例时必须填写的信息。
    //它告诉Vulkan的驱动程序需要使用的全局扩展和校验层。全局是指这里的设置对于整个应用程序都有效，而不仅仅对一个设备有效。
    VkInstanceCreateInfo createInfo = {};
    handleCreateInfo(appInfo, createInfo);

    //创建实例
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("创建实例失败");
    }
}

void HelloTriangleApplication::handleAppInfo(VkApplicationInfo& appInfo)
{
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
}

void HelloTriangleApplication::handleCreateInfo(const VkApplicationInfo& appInfo , VkInstanceCreateInfo& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    //将所需的拓展封装进getRequiredExtensions
    auto glfwExtensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
    createInfo.ppEnabledExtensionNames = glfwExtensions.data();

    //校验层信息
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        handleCreateInfo_DebugMessager(debugCreateInfo);
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

bool HelloTriangleApplication::checkValidationLayerSupport()
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

std::vector<const char*> HelloTriangleApplication::getRequiredExtensions()
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

void HelloTriangleApplication::handleCreateInfo_DebugMessager(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}

void HelloTriangleApplication::createDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    handleCreateInfo_DebugMessager(createInfo);

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
VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback
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

void HelloTriangleApplication::choosePhysicalDevice()
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
        if (!CheckQueueFamilies(device))
            continue;

        int score = CalculateScore(device);
        if (score > maxScore)
        {
            maxScore = score;
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

int HelloTriangleApplication::FindQueueFamilies(VkPhysicalDevice device , VkQueueFlagBits queueFlags)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & queueFlags)
        {
            return i;
        }
        i++;
    }
    return -1;
}

bool HelloTriangleApplication::CheckQueueFamilies(VkPhysicalDevice device)
{
    int index = FindQueueFamilies(device, VK_QUEUE_GRAPHICS_BIT);
    return index != -1;
}

void HelloTriangleApplication::CreateLogicalDevice()
{
    int queueFamilyIndex = FindQueueFamilies(m_PhysicalDevice, VK_QUEUE_GRAPHICS_BIT);
    
}

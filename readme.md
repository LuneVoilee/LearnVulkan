# Learn Vulkan

纵使身陷996福报，忙中偷闲试开新坑。  
钱塘江上潮信来，今日方知我是我！

## 主体循环

``` C++
void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }
```

窗口调用GLFW库，跟 **Vulkan** 本身无关，不再赘述。

## Vlukan初始化

### 创建实例

首先创建一个实例来初始化Vulkan库。

#### 简述流程

- 如果启用了校验层，检查你要使用的校验层是否合法
- 填充 **应用程序** 相关信息（非必须）
- 填充 **创建实例** 相关信息 （如果启用了校验层，也包含校验层信息）
- 根据所填信息创建实例

```C++
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
```

### 启用校验层

#### 校验层

***

Vulkan为了尽可能降低开销，默认情况下提供的错误检查功能非常有限。很多很基本的错误都没有被Vulkan显式地处理，遇到错误程序会直接崩溃或者发生未被明确定义的行为。

又因为Vulkan需要我们显式地定义每一个操作，所以就很容易在使用过程中产生一些小错误，比如使用了一个新的GPU特性，却忘记在逻辑设备创建时请求这一特性。

因此，Vulkan引入了校验层来优雅地解决错误检查问题。校验层是一个可选的可以用来在Vulkan API函数调用上进行附加操作的组件。
***
校验层常被用来做下面的工作：

- 检测参数值是否合法

- 追踪对象的创建和清除操作，发现资源泄漏问题

- 追踪调用来自的线程，检测是否线程安全

- 将API调用和调用的参数写入日志

- 追踪API调用进行分析和回放

***

Vulkan可以使用两种不同类型的校验层：**实例校验层** 和设备校验层。

实例校验层只检查和全局Vulkan对象相关的调用，比如Vulkan实例。设备校验层只检查和特定GPU相关的调用。  
设备校验层现在已经不推荐使用，也就是说，应该使用实例校验层来检测所有的Vulkan调用。

***

💡 这也是为什么我们在第一步创建实例的Info时，需要填入校验层的相关信息。

因为我们启用实例校验层实际上也是创建实例的一部分，只是由于篇幅原因，我们把启用校验层单开一个标题。

***

#### 简述流程

- 填充 **创建校验层** 相关信息。（之前实例的CreateInfo也使用的是同一个信息）
- 根据 实例 ， 创建信息 ， 回调函数 创建DebugMessenger

~~~ C++
void HelloTriangleApplication::createDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    handleCreateInfo_DebugMessager(createInfo);

    CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_Messenger);
}
~~~

#### 回调函数

回调函数返回了一个布尔值，用来表示引发校验层处理的Vulkan API调用是否被中断。  
如果返回值为true，对应Vulkan API调用就会返回VK_ERROR_VALIDATION_FAILED_EXT错误代码。  
通常，只在测试校验层本身时会返回true，其余情况下，回调函数应该返回VK_FALSE。

~~~ C++
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
~~~

这些参数在Messager的CreateInfo中也填充过了。

- 第一个参数messageSeverity指定消息的级别：

    - VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT：诊断信息

    - VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT：资源创建之类的信息

    - VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT：警告信息

    - VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT：不合法和可能造成崩溃的操作信息

  可以使用比较运算符来过滤处理一定级别以上的调试信息：
    ~~~ C++
    if (messageSeverity >=VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) 
    {
    // Message is important enough to show
    }
    ~~~

- 第二个参数MessageType：
    - VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT：发生了一些与规范和性能无关的事件

    - VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT：出现了违反规范的情况或发生了一个可能的错误

    - VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT：进行了可能影响Vulkan性能的行为

- 第三个参数pCallbackData参数，只有这个在先前并没有在CreateInfo里填充过，它是一个指向CallbackData结构体的指针，结构体的部分重要信息：

    - pMessage：一个以null结尾的包含调试信息的字符串

    - pObjects：存储有和消息相关的Vulkan对象句柄的数组

    - objectCount：数组中的对象个数
- 最后一个参数pUserData是一个指向用户自定义数据的指针

    - 是一个 用户自定义指针，在创建 Messenger 时通过 CreateInfo 传递给 Vulkan。它的主要作用是让用户可以在回调函数中访问外部的上下文信息

    - 在实际使用中，pUserData 可以指向任何用户定义的结构体或变量，回调函数在被调用时，Vulkan 会将这个指针原样传递给回调函数，供你在回调中使用

### 检查物理设备

物理设备指的就是显卡。

创建VkInstance后，我们可以查询系统中的显卡设备是否支持我们需要的特性。Vulkan允许我们选择任意数量的显卡设备，并能够同时使用它们，但在这里，我们只使用第一个满足我们需求的显卡设备。

#### 队列族

队列族是对物理设备上的某种功能性资源的抽象。
每个物理设备提供若干队列族，每个队列族支持特定类型的操作，比如：

- 图形操作（Graphics）
- 计算操作（Compute）
- 传输操作（Transfer，内存传输）

**每个物理设备包含多个队列族，每个队列族里的所有队列都是一样的。**

Vulkan的几乎所有操作，从绘制到加载纹理都需要将操作指令提交给一个队列，然后才能执行。

#### 简述流程

- 获取计算机的物理设备列表
- 获取物理设备的Properties，Features和队列族等相关信息，并按需进行检测

### 创建逻辑设备

逻辑设备是与物理设备交互的接口。一个物理设备可以有多个逻辑设备。

每个逻辑设备在创建时需要指明 **需要使用的队列族** 及 **每个队列族里所需队列的数量**。

#### 简述流程

- 填充 **所需队列** 的相关信息
- 填充 **设备特性** 的相关信息
- 填充 **创建逻辑设备** 的相关信息
- 创建逻辑设备
- 获取队列（逻辑设备->队列族索引->队列索引）

✨需要强调的是：

- 队列是创建逻辑设备时被创建的，因此第五步是获取而非创建。
- vkGetDeviceQueue的第三个参数，代表队列在逻辑设备的索引。  
  比如：逻辑设备有队列族1（优先级1.0），队列族2（优先级0.5），每个各请求分配了2个队列。  
  此时逻辑设备的索引3就代表 队列族2的第1个队列

~~~C++
void HelloTriangleApplication::CreateLogicalDevice()
{
    //创建逻辑设备需要先创建队列
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    float                   queuePriority = 1.0f;
    int                     queueFamilyIndex = FindQueueFamilies(m_PhysicalDevice, VK_QUEUE_GRAPHICS_BIT);
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

    vkGetDeviceQueue(m_Device, queueFamilyIndex, 0, &m_GraphicsQueue);
}
~~~

### 创建窗口表面


窗口表面（surface）是 Vulkan API 与不同操作系统窗口系统（如 Windows、Linux 和 macOS）之间的桥梁。它为不同平台的窗口提供了一个抽象层，使开发者可以通过简单的操作实现跨平台的图形输出。窗口表面的核心功能是为 Vulkan 渲染结果提供显示目标。

#### 简述流程
- 通过glfw的接口            `glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface)`可以直接创建窗口表面。
- 在之前的队列族检测中，通过如下接口：  
    ~~~C++
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
    ~~~
    检测出能够呈现图像到窗口表面的队列族。
- 获取呈现队列（上一节内容）

### 交换链
交换链，用于管理图像的呈现和显示。它负责在应用程序和显示设备之间进行图像的交换，使得绘制的内容可以最终显示在屏幕上。

#### 简述流程
- 检测设备拓展是否支持交换链：  

    通过`vkEnumerateDeviceExtensionProperties`获取所有支持的接口，然后遍历查询是否含有`VK_KHR_SWAPCHAIN_EXTENSION_NAME`。  

    实际上，如果设备支持呈现队列，那么它就一定支持交换链。但我们最好还是显式地进行交换链扩展的检测，然后显式地启用交换链扩展。  

    同时，逻辑设备的创建信息需要修改`enabledExtensionCount`和`ppEnabledExtensionNames`
- 配置交换链支持细节
    ~~~C++
    //存储查询信息的结构体
    struct SwapChainSupportDetails 
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    ~~~
    每个交换链有三种和表面相关的最基本的属性需要我们检查：

    - 基础表面特性

        `vkGetPhysicalDeviceSurfaceCapabilitiesKHR`

        通过参数返回一个结构体`VkSurfaceCapabilitiesKHR`，存放了交换链的最小/最大图像数量，最小/最大图像宽度、高度等信息。

        一般而言，交换链的分辨率就是最终窗口的分辨率。我们通过`VkSurfaceCapabilitiesKHR`里的`currentExtent`,`minImageExtent`,`minImageExtent`和glfw窗口大小等信息设置交换链的分辨率。
        ~~~C++
        VkExtent2D HelloTriangleApplication::ChooseSwapResolution(const VkSurfaceCapabilitiesKHR& capabilities)
        {
            //Vulkan通过currentExtent成员变量来告知适合我们窗口的交换范围。
            //一些窗口系统会使用一个特殊值，uint32_t变量类型的最大值，表示允许我们自己选择对于窗口最合适的交换范围
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            {
                return capabilities.currentExtent;
            }
        
            VkExtent2D actualExtent;
            actualExtent.width = Math::Clamp(Width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = Math::Clamp(Height，capabilities.minImageExtent.height,capabilities.c.height);

            return actualExtent;
        }
        ~~~
    - 表面格式

        `vkGetPhysicalDeviceSurfaceFormatsKHR`

        第四个参数返回VkSurfaceFormatKHR类型的数组，表示设备支持的所有表面格式，每个VkSurfaceFormatKHR包含了一个format（像素格式）和一个colorSpace（颜色空间）。

        - 像素格式
            - 颜色格式
                - VK_FORMAT_R8G8B8A8_UNORM:

                        每个像素由 4 个 8 位通道（红、绿、蓝、alpha）组成。

                        UNORM 表示颜色值是 [0, 1] 范围内的线性归一化值。

                - VK_FORMAT_B8G8R8A8_SRGB:

                        类似，但颜色通道顺序为蓝、绿、红、alpha。

                        SRGB 表示使用标准 RGB gamma 校正。

            - 深度格式
                - VK_FORMAT_D24_UNORM_S8_UINT:

                        24 位用于深度值，8 位用于模板值（Stencil）。


                - VK_FORMAT_D32_SFLOAT:

                        32 位浮点深度值。

            - 其他特殊格式：

                - VK_FORMAT_R5G6B5_UNORM_PACK16:
                
                        16 位 RGB 格式（5 位红，6 位绿，5 位蓝）。

                - VK_FORMAT_A2B10G10R10_UNORM_PACK32: 
                
                        32 位压缩 RGB 和 Alpha。
        - 颜色空间

            colorSpace 定义了如何解释存储在图像中的颜色数据。它主要描述了颜色值的范围和校正方式（如是否使用 gamma 校正）。

            - VK_COLOR_SPACE_SRGB_NONLINEAR_KHR：

                    默认颜色空间，表示使用标准 sRGB gamma 校正
                    sRGB 是一种非线性颜色空间，优化了人眼对亮度变化的感知

            - VK_COLOR_SPACE_LINEAR_KHR：

                    线性颜色空间。
                    没有应用 gamma 校正，直接处理线性光照强度

            - VK_COLOR_SPACE_HDR10_ST2084_EXT（HDR 支持）：

                    用于支持高动态范围（HDR）显示
                    提供更高亮度范围、更细腻的对比度

            - VK_COLOR_SPACE_BT709_LINEAR_EXT 和 VK_COLOR_SPACE_BT2020_LINEAR_EXT：

                    用于高级显示设备，如电视和影院设备

        我们选择使用VK_FORMAT_B8G8R8A8_UNORM和VK_COLOR_SPACE_SRGB_NONLINEAR_KHR。
        ~~~C++
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&   
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            {
                return availableFormat;
            }
        }
        ~~~

    - 可用的呈现模式

        `vkGetPhysicalDeviceSurfacePresentModesKHR`

        通过参数返回VkSurfaceFormatKHR类型的数组，表示设备支持的所有呈现模式。

        - VK_PRESENT_MODE_IMMEDIATE_KHR（立即模式）

            描述：

                应用程序画完一帧，就直接把它提交到屏幕上，不管显示器是不是准备好。

            可能问题：
            
                显示器可能还在显示上一帧的部分内容，就接着展示新的一帧了，显示器上同时出现多帧的画面，即为画面撕裂。

            适合场景：
            
                不在意画面撕裂但追求最低延迟的场景，比如一些工具软件。
            

        - VK_PRESENT_MODE_FIFO_KHR（队列模式， V-Sync）

            描述：

                画面按照先进先出的顺序排队，显示器每刷新一次（垂直同步）就从队列里拿一帧。如果队列满了，应用程序需要等前面的帧被显示才能提交新帧。

            问题：

                如果显卡无法按时生成一帧图像（比如 60 FPS 显示器上帧率低于 60），会导致重复显示上一帧，产生明显的卡顿感。

            适合场景：

                普通游戏，注重画面稳定性。

        - VK_PRESENT_MODE_FIFO_RELAXED_KHR（放松队列模式）

            描述：

                和上面的队列模式差不多，但放松了对垂直同步的严格要求。如果应用程序在某个垂直同步周期中没有提交新帧（队列为空），但在下一个垂直同步前提交了新帧，那么这帧会立即显示，因此可能会导致撕裂。
            效果：

                前两者的折中，减少延迟，但可能出现撕裂。

            适合场景：

                偶尔帧率不稳定的应用，比如非高强度的实时渲染。

        - VK_PRESENT_MODE_MAILBOX_KHR（邮箱模式，三倍缓冲）

            描述：

                三倍缓冲使用三个缓冲区：
                第一个缓冲区供显示器读取（显示的帧）。
                第二个缓冲区存储已经渲染好的下一帧，等待显示。
                第三个缓冲区供显卡继续渲染新的一帧。

                假设第一个缓冲区正在显示frame1，第二个缓冲区存储frame2，
                若此时第三个缓冲区里frame3已经被渲染好了，
                则第二个缓冲区会将frame2替换为frame3。可以保证显示的下一帧始终是最新的。

            效果：
                
                只要算力足够，没有撕裂，延迟更低。

            适合场景：

                追求流畅画面和低延迟的高性能游戏，比如 FPS 和动作游戏。
            
        上面四种呈现模式，只有VK_PRESENT_MODE_FIFO_KHR模式保证一定可用，但许多驱动程序对VK_PRESENT_MODE_FIFO_KHR呈现模式的支持还不够好。

        我们选择呈现模式的优先级：
        `VK_PRESENT_MODE_MAILBOX_KHR` >
        `VK_PRESENT_MODE_IMMEDIATE_KHR` > `VK_PRESENT_MODE_FIFO_KHR`

- 创建交换链
    ~~~C++
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;

    //imageCount：能够支持的缓冲区的个数
    //使用minImageCount + 1来支持三倍缓冲
    uint32_t imageCount = swapChainDetails.capabilities.minImageCount + 1;
    //maxImageCount的值为0表明，可以使用任意数量的缓冲区。所以只有maxImageCount的值大于0的时候需要检查。
    if (swapChainDetails.capabilities.maxImageCount > 0 && imageCount > swapChainDetails.capabilities.maxImageCount)
    {
        imageCount = swapChainDetails.capabilities.maxImageCount;
    }
    createInfo.minImageCount = imageCount;

    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;

    //mageArrayLayers成员变量用于指定每个图像所包含的层次。对于非立体3D应用,它的值为1。但对于VR等应用程序来说，会使用更多的层次。
    createInfo.imageArrayLayers = 1;

    //imageUsage成员变量用于指定我们将在图像上进行怎样的操作。我们在图像上进行绘制操作，也就是将图像作为一个颜色附着来使用。
    //如果需要对图像进行后期处理之类的操作，可以使用VK_IMAGE_USAGE_TRANSFER_DST_BIT作为imageUsage成员变量的值，让交换链图像可以作为传输的目的图像。
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
    createInfo.clipped = VK_TRUE;

    //最后是oldSwapchain成员变量，需要指定它，是因为应用程序在运行过程中交换链可能会失效。比如，改变窗口大小后，交换链需要重建，重建时需要之前的交换链
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    ~~~

- 获取交换链图像的图像句柄

    经典操作：

    ~~~C++
        vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &createInfo.minImageCount, nullptr);
        m_SwapChainImages.resize(createInfo.minImageCount);
        vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &createInfo.minImageCount, m_SwapChainImages.data());
    ~~~


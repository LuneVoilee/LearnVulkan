# Learn Vulkan

纵使身陷996福报生活，忙中偷闲试开新坑。  
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
- 填充 **创建** 相关信息 （如果启用了校验层，也包含校验层信息）
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

💡  这也是为什么我们在第一步创建实例的Info时，需要填入校验层的相关信息。

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
-  最后一个参数pUserData是一个指向用户自定义数据的指针

    -  是一个 用户自定义指针，在创建 Messenger 时通过 CreateInfo 传递给 Vulkan。它的主要作用是让用户可以在回调函数中访问外部的上下文信息

    - 在实际使用中，pUserData 可以指向任何用户定义的结构体或变量，回调函数在被调用时，Vulkan 会将这个指针原样传递给回调函数，供你在回调中使用
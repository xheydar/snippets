#include "v_device.hpp"



namespace vengine
{ 
    void v_device::create_instance()
    {
        std::cout << "Creating Instance" << std::endl;
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan Headless Engine";
        appInfo.pEngineName = "None";
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;

#ifdef DEBUG

	    std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

        uint32_t instanceLayerCount;
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        std::vector<VkLayerProperties> instanceLayers(instanceLayerCount);
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayers.data());

        bool layersAvailable = true;

        for (auto layerName : validationLayers) {
            bool layerAvailable = false;
            for (auto instanceLayer : instanceLayers) {
                if (strcmp(instanceLayer.layerName, layerName) == 0) {
                    layerAvailable = true;
                    break;
                }
            }
            if (!layerAvailable) {
                layersAvailable = false;
                break;
            }
        }

        const char *validationExt = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
        if (layersAvailable) {
            instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
            instanceCreateInfo.enabledLayerCount = validationLayers.size();
            instanceCreateInfo.enabledExtensionCount = 1;
            instanceCreateInfo.ppEnabledExtensionNames = &validationExt;
        }
#else
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;
        instanceCreateInfo.enabledExtensionCount = 0;
        instanceCreateInfo.ppEnabledExtensionNames = nullptr;
#endif
        VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));

#ifdef DEBUG

        std::cout << "Setting up debug messenger" << std::endl;
        for (auto layerName : validationLayers) {
            std::cout << layerName << std::endl;
            bool layerAvailable = false;
            for (auto instanceLayer : instanceLayers) {
                if (strcmp(instanceLayer.layerName, layerName) == 0) {
                    layerAvailable = true;
                    break;
                }
            }
            if (!layerAvailable) {
                layersAvailable = false;
                break;
            }
        }

        if (layersAvailable)
        {
            VkDebugReportCallbackCreateInfoEXT debugReportCreateInfo = {};
            debugReportCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            debugReportCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
            debugReportCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugMessageCallback;

            // We have to explicitly load this function.
            PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
            assert(vkCreateDebugReportCallbackEXT);
            VK_CHECK_RESULT( vkCreateDebugReportCallbackEXT(instance, &debugReportCreateInfo, nullptr, &debugReportCallback))
        }
#endif
    }

    void v_device::pick_physical_device()
    {
        std::cout << "Picking physical device" << std::endl;
        uint32_t deviceCount = 0;
        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data()));

        if( deviceCount == 0 )
        {
            throw std::runtime_error("No suitable device was found.");
        }

        physicalDevice = physicalDevices[0];

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        LOG("GPU: %s\n", deviceProperties.deviceName);

        // Request a single graphics queue
        const float defaultQueuePriority(0.0f);
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
        {
            if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                queueFamilyIndex = i;
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = i;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &defaultQueuePriority;
                break;
            }
        }

        // Create logical device
        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device_));

        // Get a graphics queue
        vkGetDeviceQueue(device_, queueFamilyIndex, 0, &queue);

        // Command pool
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        if( vkCreateCommandPool(device_, &cmdPoolInfo, nullptr, &commandPool) != VK_SUCCESS )
        {
            throw std::runtime_error("Could not create command pool");
        }

        //vks::tools::getSupportedDepthFormat(physicalDevice, &depthFormat);
    }

    v_device::v_device()
    {
        create_instance();
        pick_physical_device();
    }

    v_device::~v_device()
    {
        vkDestroyCommandPool(device_, commandPool, nullptr);
	    vkDestroyDevice(device_, nullptr);
#ifdef DEBUG
        if( debugReportCallback )
        {
            PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
            assert(vkDestroyDebugReportCallback);
            vkDestroyDebugReportCallback(instance, debugReportCallback, nullptr);
        }
#endif
        vkDestroyInstance(instance, nullptr);
    }

    VkResult v_device::createBuffer( VkBufferUsageFlags usageFlags, 
                                     VkMemoryPropertyFlags memoryPropertyFlags, 
                                     VkBuffer *buffer, 
                                     VkDeviceMemory *memory, 
                                     VkDeviceSize size, 
                                     void *data )
    {
        // Create the buffer handle
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usageFlags;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK_RESULT(vkCreateBuffer(device_, &bufferInfo, nullptr, buffer));

        // Create the memory backing up the buffer handle
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device_, *buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = getMemoryTypeIndex( physicalDevice, memRequirements.memoryTypeBits, memoryPropertyFlags);
        VK_CHECK_RESULT(vkAllocateMemory(device_, &allocInfo, nullptr, memory));

        if (data != nullptr) 
        {
            void *mapped;
            VK_CHECK_RESULT(vkMapMemory(device_, *memory, 0, size, 0, &mapped));
            memcpy(mapped, data, size);
            vkUnmapMemory(device_, *memory);
        }

        VK_CHECK_RESULT(vkBindBufferMemory(device_, *buffer, *memory, 0));

        return VK_SUCCESS;
    }

    void v_device::createBufferNoCopy( VkDeviceSize size, 
                                       VkBufferUsageFlags usage, 
                                       VkMemoryPropertyFlags properties, 
                                       VkBuffer& buffer, 
                                       VkDeviceMemory& bufferMemory )
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK_RESULT(vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = getMemoryTypeIndex( physicalDevice, 
                                                        memRequirements.memoryTypeBits, 
                                                        properties );

        VK_CHECK_RESULT(vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory));

        vkBindBufferMemory(device_, buffer, bufferMemory, 0);
    }

    void v_device::submit_work( VkCommandBuffer& cmdBuffer )
    {
        VkSubmitInfo submitInfo = vks::initializers::submitInfo();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        VkFenceCreateInfo fenceInfo = vks::initializers::fenceCreateInfo();
        VkFence fence;
        VK_CHECK_RESULT(vkCreateFence(device_, &fenceInfo, nullptr, &fence));
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
        VK_CHECK_RESULT(vkWaitForFences(device_, 1, &fence, VK_TRUE, UINT64_MAX));
        vkDestroyFence(device_, fence, nullptr);
	    vkQueueWaitIdle(queue);
    }
}

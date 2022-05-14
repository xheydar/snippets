#pragma once

#include <iostream>
#include <vector>
#include "VulkanTools.h"
#include "base_tools.hpp"

namespace vengine
{
    class v_device
    {
        private:
            VkInstance instance;
            VkDebugReportCallbackEXT debugReportCallback{};
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            uint32_t physicalDeviceIdx;
			//VkDevice device;
			uint32_t queueFamilyIndex;
			VkQueue queue;
			VkCommandPool commandPool;
            VkDevice device_;

            void create_instance();
            void pick_physical_device();
        public:
            v_device();
            ~v_device();

            v_device( const v_device& ) = delete;
            void operator = ( const v_device& ) = delete;
            v_device( v_device&& ) = delete;
            v_device operator = ( v_device&& ) = delete;

            VkDevice& device() { return device_; };
            VkPhysicalDevice& get_physical_device() { return physicalDevice; };
            VkCommandPool& get_command_pool() { return commandPool; };
            VkQueue& get_queue() { return queue; };

            VkResult createBuffer( VkBufferUsageFlags usageFlags, 
                                   VkMemoryPropertyFlags memoryPropertyFlags, 
                                   VkBuffer *buffer, VkDeviceMemory *memory, 
                                   VkDeviceSize size, 
                                   void *data=nullptr );

            void createBufferNoCopy( VkDeviceSize size,
                                     VkBufferUsageFlags usage, 
                                     VkMemoryPropertyFlags properties, 
                                     VkBuffer& buffer, 
                                     VkDeviceMemory& bufferMemory );

            void submit_work( VkCommandBuffer& cmdBuffer ); 
    };
}

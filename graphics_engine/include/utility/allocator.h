#pragma once
#include "vma/vk_mem_alloc.h"
#include "utility/logger.h"
#include "renderer/device.h"
#include "renderer/instance.h"

class DeviceMemoryManager {
public:
    void initialize(Device* device, Instance* instance);
    void cleanup();

    Device* device;
    Instance* instance;
    VmaAllocator allocator;
};

#pragma once
#include "vk_mem_alloc.h"
#include "logger.h"
#include "device.h"
#include "instance.h"

class DeviceMemoryManager {
public:
    void initialize(Device* device, Instance* instance);
    void cleanup();

    Device* device;
    Instance* instance;
    VmaAllocator allocator;
};

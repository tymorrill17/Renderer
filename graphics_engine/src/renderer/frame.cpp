#include "renderer/frame.h"
#include "renderer/command.h"

Frame::Frame(Device* device) :
	_presentSemaphore(device),
	_renderSemaphore(device),
	_renderFence(device, VK_FENCE_CREATE_SIGNALED_BIT)
{}

Frame::Frame(Frame&& other) noexcept :
    _presentSemaphore(std::move(other._presentSemaphore)),
    _renderSemaphore(std::move(other._renderSemaphore)),
    _renderFence(std::move(other._renderFence)) {
}

Frame& Frame::operator=(Frame&& other) noexcept {
    if (this != &other) {
        _presentSemaphore = std::move(other._presentSemaphore);
        _renderSemaphore = std::move(other._renderSemaphore);
        _renderFence = std::move(other._renderFence);
    }
    return *this;
}


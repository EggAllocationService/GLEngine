#include "pipeline/wgpu/TransferManager.h"
#include "pipeline/wgpu/WGPURenderer.h"
#include <format>

using namespace glengine::pipeline::wgpu;

struct CallbackInfo {
	std::unique_ptr<StagingBuffer> buffer;
	TransferManager* manager;
};


static void transferSessionCallback(WGPUMapAsyncStatus status, WGPUStringView message, void* userdata1, void* userdata2) {
	auto info = reinterpret_cast<CallbackInfo*>(userdata1);

	if (status == WGPUMapAsyncStatus_Success) {
		info->manager->Return(std::move(info->buffer));
	}

	delete info;
}

TransferSession::TransferSession(std::string name, TransferManager* manager, WGPUDevice device, WGPUQueue queue, std::unique_ptr<StagingBuffer> buffer) {
	this->manager = manager;
	this->device = device;
	this->queue = queue;
	this->buffer = std::move(buffer);
	this->usedCapacity = 0;
	this->name = name;
}

void TransferSession::Transfer(WGPUBuffer target, unsigned int offset, void* data, unsigned int length) {
	transfers.push_back(TransferInfo{
			.target = target,
			.data = data,
			.length = length,
			.offset = offset,
			.sourceOffset = usedCapacity
		});
	usedCapacity += length;
}

void TransferSession::Commit() {
	if (usedCapacity > buffer->capacity) {
		manager->Return(std::move(buffer));
		buffer = manager->Take(usedCapacity);
	}

	WGPUCommandEncoderDescriptor desc = {
		.nextInChain = nullptr,
		.label = {
			.data = name.data(),
			.length = name.length()
		}
	};
	
	for (const auto& transfer : this->transfers) {
		memcpy(reinterpret_cast<char*>(buffer->mappedRange) + transfer.sourceOffset, transfer.data, transfer.length);
	}

	wgpuBufferUnmap(buffer->buffer);

	auto encoder = wgpuDeviceCreateCommandEncoder(device, &desc);
	for (const auto& transfer : this->transfers) {
		wgpuCommandEncoderCopyBufferToBuffer(encoder, buffer->buffer, transfer.sourceOffset, transfer.target, transfer.offset, transfer.length);
	}

	WGPUCommandBufferDescriptor bufDesc = {
		.nextInChain = nullptr,
		.label = {
			.data = buffer->name.data(),
			.length = buffer->name.length()
		}
	};
	auto commandBuf = wgpuCommandEncoderFinish(encoder, &bufDesc);
	wgpuCommandEncoderRelease(encoder);
	
	wgpuQueueSubmit(queue, 1, &commandBuf);
	wgpuCommandBufferRelease(commandBuf);

	auto cbi = new CallbackInfo();

	auto targetBuf = buffer->buffer;
	auto targetCap = buffer->capacity;

	cbi->buffer = std::move(buffer);
	cbi->manager = manager;

	WGPUBufferMapCallbackInfo cb = {
		.nextInChain = nullptr,
		.mode = WGPUCallbackMode_AllowSpontaneous,
		.callback = transferSessionCallback,
		.userdata1 = cbi
	};
	wgpuBufferMapAsync(targetBuf, WGPUMapMode_Write, 0, targetCap, cb) ;
}

TransferManager::TransferManager(WGPURenderer* renderer) {
	device = renderer->GetDevice();
	queue = wgpuDeviceGetQueue(device);

	for (int i = 0; i < 5; i++) {
		// make 5 small transfer buffers
		auto buf = allocateNew(1024);
		availableBuffers.push_back(std::move(buf));
	}

	for (int i = 0; i < 5; i++) {
		// make 5 large transfer buffers
		auto buf = allocateNew(65536);
		availableBuffers.push_back(std::move(buf));
	}
}

std::unique_ptr<TransferSession> TransferManager::CreateSession(std::string name, unsigned int estimatedTotalSize) {
	return std::make_unique<TransferSession>(std::format("Ring Transfer: {}", name), this, device, queue, Take(estimatedTotalSize));
}

void TransferManager::Return(std::unique_ptr<StagingBuffer> buffer) {
	std::lock_guard<std::mutex> lock(ringMutex);
	availableBuffers.push_back(std::move(buffer));
}

std::unique_ptr<StagingBuffer> TransferManager::Take(unsigned int minSize) {
	std::lock_guard<std::mutex> lock(ringMutex);
	// find buffer closest to minSize without going under

	auto best_it = availableBuffers.end();

	// Iterate through the list to find the optimal candidate
	for (auto it = availableBuffers.begin(); it != availableBuffers.end(); ++it) {
		if ((*it)->capacity >= minSize) {
			// If we haven't found a candidate yet, OR this candidate is closer to the target
			if (best_it == availableBuffers.end() || (*it)->capacity < (*best_it)->capacity) {
				best_it = it;

				// Optional optimization: If we find an exact match, we can stop searching immediately
				if ((*best_it)->capacity == minSize) {
					break;
				}
			}
		}
	}

	// If we found a valid item, extract it and remove the empty node from the list
	if (best_it != availableBuffers.end()) {
		std::unique_ptr<StagingBuffer> result = std::move(*best_it);
		availableBuffers.erase(best_it);
		return result;
	}

	// allocate a new buffer with double the capacity in this case
	return allocateNew(minSize * 2);
}

std::unique_ptr<StagingBuffer> TransferManager::allocateNew(unsigned int capacity) {
	auto newItem = std::make_unique<StagingBuffer>();
	newItem->capacity = capacity;
	newItem->name = std::format("GLEngine transfer buffer #{}", ++bufferCounter);


	WGPUBufferDescriptor desc = {
		.nextInChain = nullptr,
		.label = {
			.data = newItem->name.data(),
			.length = newItem->name.length()
		},
		.usage = WGPUBufferUsage_MapWrite | WGPUBufferUsage_CopySrc,
		.size = capacity,
		.mappedAtCreation = true
	};
	newItem->buffer = wgpuDeviceCreateBuffer(device, &desc);
	newItem->mappedRange = wgpuBufferGetMappedRange(newItem->buffer, 0, capacity);
	return newItem;
}
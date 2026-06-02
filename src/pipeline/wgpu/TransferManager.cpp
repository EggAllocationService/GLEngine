#include "pipeline/wgpu/TransferManager.h"
#include "pipeline/wgpu/WGPURenderer.h"
#include <format>
#include <cstring>

using namespace glengine::pipeline::wgpu;

struct CallbackInfo {
	std::unique_ptr<StagingBuffer> buffer;
	TransferManager* manager;
};


static void transferSessionCallback(WGPUMapAsyncStatus status, WGPUStringView message, void* userdata1, void* userdata2) {
	auto info = reinterpret_cast<CallbackInfo*>(userdata1);


	if (status == WGPUMapAsyncStatus_Success) {
		info->buffer->mappedRange = wgpuBufferGetMappedRange(info->buffer->buffer, 0, info->buffer->capacity);
		info->manager->Return(std::move(info->buffer));
	}

	delete info;
}

TransferSession::TransferSession(std::string name, TransferManager* manager, WGPUDevice device, WGPUQueue queue, std::unique_ptr<StagingBuffer> buffer) {
	this->manager = manager;
	this->device = device;
	this->queue = queue;
	this->buffers.push_back(std::move(buffer));
	this->usedCapacity.push_back(0);
	this->name = name;
}

void TransferSession::Transfer(WGPUBuffer target, unsigned int offset, void* data, unsigned int length) {
	bool fitIntoExisting = false;
	for (unsigned int i = 0; i < buffers.size(); i++) {
		// find a buffer which can fit the request
		if (buffers[i]->capacity - usedCapacity[i] > length) {
			// this buffer has enough room!
			transfers.push_back(TransferInfo{
				.target = target,
				.data = data,
				.length = length,
				.offset = offset,
				.sourceIndex = i,
				.sourceOffset = usedCapacity[i]
			});
			usedCapacity[i] += length;
			fitIntoExisting = true;
		}
	}

	if (!fitIntoExisting) {
		// we don't have room in any of the existing buffers
		// if we can do this without allocating new space, then do that and add another buffer to the transfer
		// otherwise, replace the last buffer
		auto existing = manager->TakeWithoutAllocating(length);
		if (existing == nullptr) {
			unsigned int lastIdx = buffers.size() - 1;
			// we'd have to allocate a new buffer anyways. In this case, allocate a new one and release the last buffer back into the pool
			auto newBuf = manager->AllocateNew(usedCapacity[lastIdx] + length * 2);

			// swap the new one into the last slot
			std::swap(buffers[lastIdx], newBuf);
			manager->Return(std::move(newBuf));

			// add a new transfer
			transfers.push_back(TransferInfo{
				.target = target,
				.data = data,
				.length = length,
				.offset = offset,
				.sourceIndex = lastIdx,
				.sourceOffset = usedCapacity[lastIdx]
			});
			usedCapacity[lastIdx] += length;
		} else {
			// we found another buffer to use for this transfer, so just use that
			unsigned int newIndex = buffers.size();
			buffers.push_back(std::move(existing));
			usedCapacity.push_back(length);
			transfers.push_back(TransferInfo{
				.target = target,
				.data = data,
				.length = length,
				.offset = offset,
				.sourceIndex = newIndex,
				.sourceOffset = 0
			});
		}
	}
}

void TransferSession::Commit() {
	WGPUCommandEncoderDescriptor desc = {
		.nextInChain = nullptr,
		.label = {
			.data = name.data(),
			.length = name.length()
		}
	};
	
	for (const auto& transfer : this->transfers) {
		memcpy(reinterpret_cast<char*>(buffers[transfer.sourceIndex]->mappedRange) + transfer.sourceOffset, transfer.data, transfer.length);
	}

	for (const auto& buffer: this->buffers) {
		wgpuBufferUnmap(buffer->buffer);
	}

	auto encoder = wgpuDeviceCreateCommandEncoder(device, &desc);
	for (const auto& transfer : this->transfers) {
		wgpuCommandEncoderCopyBufferToBuffer(encoder, buffers[transfer.sourceIndex]->buffer, transfer.sourceOffset, transfer.target, transfer.offset, transfer.length);
	}

	WGPUCommandBufferDescriptor bufDesc = {
		.nextInChain = nullptr,
		.label = {
			.data = "Transfer Session",
			.length = WGPU_STRLEN
		}
	};
	auto commandBuf = wgpuCommandEncoderFinish(encoder, &bufDesc);
	wgpuCommandEncoderRelease(encoder);
	
	wgpuQueueSubmit(queue, 1, &commandBuf);
	wgpuCommandBufferRelease(commandBuf);


	for (auto&buffer: buffers) {
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
}

TransferManager::TransferManager(WGPURenderer* renderer) {
	this->bufferCounter = 0;
	device = renderer->GetDevice();
	queue = wgpuDeviceGetQueue(device);

	for (int i = 0; i < 5; i++) {
		// make 5 small transfer buffers
		auto buf = AllocateNew(1024);
		availableBuffers.push_back(std::move(buf));
	}

	for (int i = 0; i < 5; i++) {
		// make 5 large transfer buffers
		auto buf = AllocateNew(65536);
		availableBuffers.push_back(std::move(buf));
	}
}

std::unique_ptr<TransferSession> TransferManager::CreateSession(std::string name, unsigned int estimatedTotalSize) {
	return std::make_unique<TransferSession>(std::format("Ring Transfer: {}", name), this, device, queue, Take(estimatedTotalSize));
}

void TransferManager::Transfer(WGPUBuffer target, unsigned int offset, void* data, unsigned int length) {
	auto buf = Take(length);
	memcpy(buf->mappedRange, data, length);

	wgpuBufferUnmap(buf->buffer);

	WGPUCommandEncoderDescriptor encDesc = {
		.nextInChain = nullptr,
		.label {
			.data = "Anonymous Ring Transfer",
			.length = WGPU_STRLEN
		}
	};

	auto encoder = wgpuDeviceCreateCommandEncoder(device, &encDesc);
	wgpuCommandEncoderCopyBufferToBuffer(encoder, buf->buffer, 0, target, offset, length);

	WGPUCommandBufferDescriptor cmdDesc = {
		.nextInChain = nullptr,
		.label {
			.data = "Anonymous Ring Transfer",
			.length = WGPU_STRLEN
		}
	};

	auto cmd = wgpuCommandEncoderFinish(encoder, &cmdDesc);
	wgpuCommandEncoderRelease(encoder);

	wgpuQueueSubmit(queue, 1, &cmd);
	wgpuCommandBufferRelease(cmd);


	auto srcBuf = buf->buffer;
	auto srcCap = buf->capacity;

	auto cbi = new CallbackInfo();
	cbi->buffer = std::move(buf);
	cbi->manager = this;

	WGPUBufferMapCallbackInfo cb = {
		.nextInChain = nullptr,
		.mode = WGPUCallbackMode_AllowSpontaneous,
		.callback = transferSessionCallback,
		.userdata1 = cbi
	};
	
	wgpuBufferMapAsync(srcBuf, WGPUMapMode_Write, 0, srcCap, cb);
}

void TransferManager::Return(std::unique_ptr<StagingBuffer> buffer) {
	std::lock_guard<std::mutex> lock(ringMutex);
	availableBuffers.push_back(std::move(buffer));
}

std::unique_ptr<StagingBuffer> TransferManager::TakeWithoutAllocating(unsigned int minSize) {
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

	if (best_it == availableBuffers.end()) {
		return nullptr;
	}
	auto result = std::move(*best_it);
	availableBuffers.erase(best_it);
	return std::move(result);
}

std::unique_ptr<StagingBuffer> TransferManager::Take(unsigned int minSize) {
	auto potentialExisting = TakeWithoutAllocating(minSize);

	// allocate a new buffer with double the capacity in this case
	return potentialExisting == nullptr ? AllocateNew(minSize * 2) : std::move(potentialExisting);
}

std::unique_ptr<StagingBuffer> TransferManager::AllocateNew(unsigned int capacity) {
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
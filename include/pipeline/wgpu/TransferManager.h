#pragma once

#include "webgpu/wgpu.h"
#include "glengine_export.h"
#include <vector>
#include <memory>
#include <string>
#include <mutex>

namespace glengine::pipeline::wgpu {
	class WGPURenderer;
	class TransferManager;

	struct GLENGINE_EXPORT TransferInfo {
		WGPUBuffer target;
		const void* data;
		unsigned int length;
		unsigned int offset;

		unsigned int sourceIndex;
		unsigned int sourceOffset;
	};

	struct GLENGINE_EXPORT StagingBuffer {
		std::string name;
		WGPUBuffer buffer;
		void* mappedRange;
		unsigned int capacity;
	};

	class TransferSession {

	public:
		TransferSession(std::string name, TransferManager* manager, WGPUDevice device, WGPUQueue queue, std::unique_ptr<StagingBuffer> buffer);

		GLENGINE_EXPORT void Transfer(WGPUBuffer target, unsigned int offset, const void* data, unsigned int length);

		GLENGINE_EXPORT void Commit();
	private:
		std::vector<TransferInfo> transfers;
		std::string name;
		std::vector<std::unique_ptr<StagingBuffer>> buffers;
		WGPUDevice device;
		WGPUQueue queue;
		TransferManager* manager;
		std::vector<unsigned int> usedCapacity;
	};

	class TransferManager {
	public:
		TransferManager(WGPURenderer* renderer);

		GLENGINE_EXPORT std::unique_ptr<TransferSession> CreateSession(std::string name, unsigned int estimatedTotalSize);

		GLENGINE_EXPORT void Transfer(WGPUBuffer target, unsigned int offset, const void* data, unsigned int length);

		GLENGINE_EXPORT void Return(std::unique_ptr<StagingBuffer> buffer);
		GLENGINE_EXPORT std::unique_ptr<StagingBuffer> TakeWithoutAllocating(unsigned int minSize);
		GLENGINE_EXPORT std::unique_ptr<StagingBuffer> Take(unsigned int minSize);
		GLENGINE_EXPORT std::unique_ptr<StagingBuffer> AllocateNew(unsigned int capacity);

	private:
		std::vector<std::unique_ptr<StagingBuffer>> availableBuffers;
		std::mutex ringMutex;
		WGPUDevice device;
		WGPUQueue queue;

		int bufferCounter;
	};
}
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
		void* data;
		unsigned int length;
		unsigned int offset;

		unsigned int sourceOffset;
	};

	struct GLENGINE_EXPORT StagingBuffer {
		std::string name;
		WGPUBuffer buffer;
		void* mappedRange;
		unsigned int capacity;
	};

	class GLENGINE_EXPORT TransferSession {

	public:
		TransferSession(std::string name, TransferManager* manager, WGPUDevice device, WGPUQueue queue, std::unique_ptr<StagingBuffer> buffer);

		void Transfer(WGPUBuffer target, unsigned int offset, void* data, unsigned int length);

		void Commit();
	private:
		std::vector<TransferInfo> transfers;
		std::string name;
		std::unique_ptr<StagingBuffer> buffer;
		WGPUDevice device;
		WGPUQueue queue;
		TransferManager* manager;
		unsigned int usedCapacity;
	};

	class GLENGINE_EXPORT TransferManager {
	public:
		TransferManager(WGPURenderer* renderer);

		std::unique_ptr<TransferSession> CreateSession(std::string name, unsigned int estimatedTotalSize);

		void Transfer(WGPUBuffer target, unsigned int offset, void* data, unsigned int length);

		void Return(std::unique_ptr<StagingBuffer> buffer);
		std::unique_ptr<StagingBuffer> Take(unsigned int minSize);
	private:
		std::unique_ptr<StagingBuffer> allocateNew(unsigned int capacity);

		std::vector<std::unique_ptr<StagingBuffer>> availableBuffers;
		std::mutex ringMutex;
		WGPUDevice device;
		WGPUQueue queue;

		int bufferCounter;
	};
}
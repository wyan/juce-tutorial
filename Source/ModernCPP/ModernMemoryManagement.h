#pragma once
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>
#include <array>
#include <new>
#include <cstdlib>

namespace ModernAudio {

// RAII-based audio buffer management
class AudioBufferManager {
public:
    explicit AudioBufferManager(size_t bufferSize, size_t numChannels)
        : bufferSize_(bufferSize), numChannels_(numChannels) {
        allocateBuffers();
    }

    // Move-only semantics for efficient resource management
    AudioBufferManager(const AudioBufferManager&) = delete;
    AudioBufferManager& operator=(const AudioBufferManager&) = delete;

    AudioBufferManager(AudioBufferManager&&) = default;
    AudioBufferManager& operator=(AudioBufferManager&&) = default;

    ~AudioBufferManager() = default; // RAII cleanup

    float* getChannelData(size_t channel) {
        if (channel < buffers_.size()) {
            return buffers_[channel].get();
        }
        return nullptr;
    }

    const float* getChannelData(size_t channel) const {
        if (channel < buffers_.size()) {
            return buffers_[channel].get();
        }
        return nullptr;
    }

    void resizeBuffers(size_t newSize) {
        std::lock_guard<std::mutex> lock(mutex_);
        bufferSize_ = newSize;
        allocateBuffers();
    }

    size_t getBufferSize() const { return bufferSize_; }
    size_t getNumChannels() const { return numChannels_; }

private:
    size_t bufferSize_;
    size_t numChannels_;
    std::vector<std::unique_ptr<float[]>> buffers_;
    mutable std::mutex mutex_;

    void allocateBuffers() {
        buffers_.clear();
        buffers_.reserve(numChannels_);

        for (size_t i = 0; i < numChannels_; ++i) {
            buffers_.emplace_back(std::make_unique<float[]>(bufferSize_));
        }
    }
};

// Simple memory pool for real-time allocations
class MemoryPool {
public:
    explicit MemoryPool(size_t poolSize)
        : poolSize_(poolSize), pool_(std::make_unique<char[]>(poolSize)),
          currentOffset_(0) {}

    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        std::lock_guard<std::mutex> lock(mutex_);

        // Align the current offset
        size_t alignedOffset = (currentOffset_ + alignment - 1) & ~(alignment - 1);

        if (alignedOffset + size > poolSize_) {
            return nullptr; // Pool exhausted
        }

        void* ptr = pool_.get() + alignedOffset;
        currentOffset_ = alignedOffset + size;
        return ptr;
    }

    void deallocate(void* ptr, size_t size) {
        // Simple pool doesn't support individual deallocation
        // In a real implementation, you'd track allocations
        (void)ptr;
        (void)size;
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        currentOffset_ = 0;
    }

    size_t getBytesUsed() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return currentOffset_;
    }

    size_t getBytesAvailable() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return poolSize_ - currentOffset_;
    }

private:
    size_t poolSize_;
    std::unique_ptr<char[]> pool_;
    size_t currentOffset_;
    mutable std::mutex mutex_;
};

// Custom allocator for real-time audio processing
template<typename T>
class RealtimeAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template<typename U>
    struct rebind {
        using other = RealtimeAllocator<U>;
    };

    RealtimeAllocator() = default;

    template<typename U>
    RealtimeAllocator(const RealtimeAllocator<U>&) noexcept {}

    pointer allocate(size_type n) {
        if (n > max_size()) {
            throw std::bad_alloc();
        }

        // Use memory pool for real-time safety
        void* ptr = getMemoryPool().allocate(n * sizeof(T), alignof(T));
        if (!ptr) {
            throw std::bad_alloc();
        }
        return static_cast<pointer>(ptr);
    }

    void deallocate(pointer p, size_type n) noexcept {
        getMemoryPool().deallocate(p, n * sizeof(T));
    }

    size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    template<typename U>
    bool operator==(const RealtimeAllocator<U>&) const noexcept {
        return true;
    }

    template<typename U>
    bool operator!=(const RealtimeAllocator<U>&) const noexcept {
        return false;
    }

private:
    static MemoryPool& getMemoryPool() {
        static MemoryPool pool(1024 * 1024); // 1MB pool
        return pool;
    }
};

// Lock-free circular buffer for real-time audio
template<typename T, size_t Size>
class LockFreeCircularBuffer {
public:
    static_assert((Size & (Size - 1)) == 0, "Size must be power of 2");

    bool push(const T& item) {
        const auto current_tail = tail_.load(std::memory_order_relaxed);
        const auto next_tail = increment(current_tail);

        if (next_tail != head_.load(std::memory_order_acquire)) {
            buffer_[current_tail] = item;
            tail_.store(next_tail, std::memory_order_release);
            return true;
        }
        return false; // Buffer full
    }

    bool pop(T& item) {
        const auto current_head = head_.load(std::memory_order_relaxed);

        if (current_head == tail_.load(std::memory_order_acquire)) {
            return false; // Buffer empty
        }

        item = buffer_[current_head];
        head_.store(increment(current_head), std::memory_order_release);
        return true;
    }

    bool empty() const {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

    bool full() const {
        const auto next_tail = increment(tail_.load(std::memory_order_acquire));
        return next_tail == head_.load(std::memory_order_acquire);
    }

    size_t size() const {
        const auto current_head = head_.load(std::memory_order_acquire);
        const auto current_tail = tail_.load(std::memory_order_acquire);
        return (current_tail - current_head) & (Size - 1);
    }

    static constexpr size_t capacity() {
        return Size - 1; // One slot reserved to distinguish full from empty
    }

private:
    static constexpr size_t increment(size_t idx) {
        return (idx + 1) & (Size - 1);
    }

    std::array<T, Size> buffer_;
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};
};

// RAII wrapper for audio device resources
class AudioDeviceRAII {
public:
    explicit AudioDeviceRAII(int deviceId) : deviceId_(deviceId), isOpen_(false) {
        // Simulate opening audio device
        if (openDevice(deviceId_)) {
            isOpen_ = true;
        }
    }

    ~AudioDeviceRAII() {
        if (isOpen_) {
            closeDevice(deviceId_);
        }
    }

    // Move-only semantics
    AudioDeviceRAII(const AudioDeviceRAII&) = delete;
    AudioDeviceRAII& operator=(const AudioDeviceRAII&) = delete;

    AudioDeviceRAII(AudioDeviceRAII&& other) noexcept
        : deviceId_(other.deviceId_), isOpen_(other.isOpen_) {
        other.isOpen_ = false;
    }

    AudioDeviceRAII& operator=(AudioDeviceRAII&& other) noexcept {
        if (this != &other) {
            if (isOpen_) {
                closeDevice(deviceId_);
            }
            deviceId_ = other.deviceId_;
            isOpen_ = other.isOpen_;
            other.isOpen_ = false;
        }
        return *this;
    }

    bool isOpen() const { return isOpen_; }
    int getDeviceId() const { return deviceId_; }

private:
    int deviceId_;
    bool isOpen_;

    bool openDevice(int id) {
        // Simulate device opening
        return id >= 0;
    }

    void closeDevice(int id) {
        // Simulate device closing
        (void)id;
    }
};

// Smart pointer for audio samples with custom deleter
class AudioSampleDeleter {
public:
    void operator()(float* ptr) const {
        // Custom aligned deallocation
        std::free(ptr);
    }
};

using AudioSamplePtr = std::unique_ptr<float[], AudioSampleDeleter>;

// Factory function for aligned audio sample allocation
inline AudioSamplePtr makeAlignedAudioBuffer(size_t numSamples, size_t alignment = 32) {
    void* ptr = nullptr;

#ifdef _WIN32
    ptr = _aligned_malloc(numSamples * sizeof(float), alignment);
#else
    if (posix_memalign(&ptr, alignment, numSamples * sizeof(float)) != 0) {
        ptr = nullptr;
    }
#endif

    if (!ptr) {
        throw std::bad_alloc();
    }

    return AudioSamplePtr(static_cast<float*>(ptr));
}

// Shared resource manager for audio plugins
template<typename ResourceType>
class SharedResourceManager {
public:
    using ResourcePtr = std::shared_ptr<ResourceType>;

    template<typename... Args>
    ResourcePtr getOrCreate(const std::string& key, Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = resources_.find(key);
        if (it != resources_.end()) {
            if (auto resource = it->second.lock()) {
                return resource;
            } else {
                // Resource expired, remove from map
                resources_.erase(it);
            }
        }

        // Create new resource
        auto resource = std::make_shared<ResourceType>(std::forward<Args>(args)...);
        resources_[key] = resource;
        return resource;
    }

    void cleanup() {
        std::lock_guard<std::mutex> lock(mutex_);

        for (auto it = resources_.begin(); it != resources_.end();) {
            if (it->second.expired()) {
                it = resources_.erase(it);
            } else {
                ++it;
            }
        }
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return resources_.size();
    }

private:
    std::map<std::string, std::weak_ptr<ResourceType>> resources_;
    mutable std::mutex mutex_;
};

// Example usage types
using AudioBufferPtr = std::unique_ptr<AudioBufferManager>;
using SharedAudioBuffer = std::shared_ptr<AudioBufferManager>;

// Factory functions
inline AudioBufferPtr makeAudioBuffer(size_t bufferSize, size_t numChannels) {
    return std::make_unique<AudioBufferManager>(bufferSize, numChannels);
}

inline SharedAudioBuffer makeSharedAudioBuffer(size_t bufferSize, size_t numChannels) {
    return std::make_shared<AudioBufferManager>(bufferSize, numChannels);
}

} // namespace ModernAudio

#pragma once
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <map>
#include <string>
#include <iostream>
#include <iomanip>
#include <atomic>
#include <mutex>

namespace ModernAudio::Profiling {

class PerformanceProfiler {
public:
    struct ProfileData {
        std::vector<double> samples;
        double totalTime = 0.0;
        size_t callCount = 0;

        double getAverage() const {
            return callCount > 0 ? totalTime / callCount : 0.0;
        }

        double getMin() const {
            return samples.empty() ? 0.0 : *std::min_element(samples.begin(), samples.end());
        }

        double getMax() const {
            return samples.empty() ? 0.0 : *std::max_element(samples.begin(), samples.end());
        }

        double getPercentile(double p) const {
            if (samples.empty()) return 0.0;

            auto sorted = samples;
            std::sort(sorted.begin(), sorted.end());

            size_t index = static_cast<size_t>(p * (sorted.size() - 1));
            return sorted[index];
        }

        double getStandardDeviation() const {
            if (samples.size() < 2) return 0.0;

            double mean = getAverage();
            double variance = 0.0;

            for (double sample : samples) {
                double diff = sample - mean;
                variance += diff * diff;
            }

            variance /= (samples.size() - 1);
            return std::sqrt(variance);
        }
    };

    class ScopedTimer {
    public:
        ScopedTimer(PerformanceProfiler& profiler, const std::string& name)
            : profiler_(profiler), name_(name),
              start_(std::chrono::high_resolution_clock::now()) {}

        ~ScopedTimer() {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double, std::micro>(end - start_);
            profiler_.addSample(name_, duration.count());
        }

    private:
        PerformanceProfiler& profiler_;
        std::string name_;
        std::chrono::high_resolution_clock::time_point start_;
    };

    void addSample(const std::string& name, double microseconds) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& data = profiles_[name];
        data.samples.push_back(microseconds);
        data.totalTime += microseconds;
        data.callCount++;

        // Keep only recent samples to avoid memory growth
        if (data.samples.size() > maxSamples_) {
            data.samples.erase(data.samples.begin());
        }
    }

    ScopedTimer createTimer(const std::string& name) {
        return ScopedTimer(*this, name);
    }

    void printReport() const {
        std::lock_guard<std::mutex> lock(mutex_);

        std::cout << "\n=== Performance Profile Report ===\n";
        std::cout << std::left << std::setw(20) << "Function"
                  << std::setw(10) << "Calls"
                  << std::setw(12) << "Avg (μs)"
                  << std::setw(12) << "Min (μs)"
                  << std::setw(12) << "Max (μs)"
                  << std::setw(12) << "95th %ile"
                  << std::setw(12) << "StdDev"
                  << "\n";
        std::cout << std::string(90, '-') << "\n";

        for (const auto& [name, data] : profiles_) {
            std::cout << std::left << std::setw(20) << name
                      << std::setw(10) << data.callCount
                      << std::setw(12) << std::fixed << std::setprecision(2) << data.getAverage()
                      << std::setw(12) << data.getMin()
                      << std::setw(12) << data.getMax()
                      << std::setw(12) << data.getPercentile(0.95)
                      << std::setw(12) << data.getStandardDeviation()
                      << "\n";
        }
        std::cout << std::string(90, '=') << "\n";
    }

    void printDetailedReport(const std::string& functionName) const {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = profiles_.find(functionName);
        if (it == profiles_.end()) {
            std::cout << "No profile data found for: " << functionName << "\n";
            return;
        }

        const auto& data = it->second;
        std::cout << "\n=== Detailed Report for " << functionName << " ===\n";
        std::cout << "Total calls: " << data.callCount << "\n";
        std::cout << "Total time: " << data.totalTime << " μs\n";
        std::cout << "Average: " << data.getAverage() << " μs\n";
        std::cout << "Min: " << data.getMin() << " μs\n";
        std::cout << "Max: " << data.getMax() << " μs\n";
        std::cout << "Standard deviation: " << data.getStandardDeviation() << " μs\n";
        std::cout << "Percentiles:\n";
        std::cout << "  50th: " << data.getPercentile(0.50) << " μs\n";
        std::cout << "  90th: " << data.getPercentile(0.90) << " μs\n";
        std::cout << "  95th: " << data.getPercentile(0.95) << " μs\n";
        std::cout << "  99th: " << data.getPercentile(0.99) << " μs\n";
        std::cout << std::string(50, '=') << "\n";
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        profiles_.clear();
    }

    ProfileData getProfileData(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = profiles_.find(name);
        return it != profiles_.end() ? it->second : ProfileData{};
    }

    std::vector<std::string> getProfiledFunctions() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> functions;
        for (const auto& [name, data] : profiles_) {
            functions.push_back(name);
        }
        return functions;
    }

private:
    std::map<std::string, ProfileData> profiles_;
    mutable std::mutex mutex_;
    static constexpr size_t maxSamples_ = 1000;
};

// Global profiler instance
inline PerformanceProfiler& getGlobalProfiler() {
    static PerformanceProfiler profiler;
    return profiler;
}

#define PROFILE_FUNCTION_GLOBAL() \
    auto _timer = getGlobalProfiler().createTimer(__FUNCTION__)

#define PROFILE_SCOPE_GLOBAL(name) \
    auto _timer = getGlobalProfiler().createTimer(name)

// Real-time safe profiler for audio threads
class RealtimeProfiler {
public:
    struct RealtimeStats {
        std::atomic<uint64_t> totalCycles{0};
        std::atomic<uint64_t> callCount{0};
        std::atomic<uint64_t> maxCycles{0};
        std::atomic<uint64_t> minCycles{UINT64_MAX};

        double getAverageMicroseconds() const {
            uint64_t calls = callCount.load();
            if (calls == 0) return 0.0;

            uint64_t total = totalCycles.load();
            return cyclesToMicroseconds(total / calls);
        }

        double getMaxMicroseconds() const {
            return cyclesToMicroseconds(maxCycles.load());
        }

        double getMinMicroseconds() const {
            uint64_t min = minCycles.load();
            return min == UINT64_MAX ? 0.0 : cyclesToMicroseconds(min);
        }

        void reset() {
            totalCycles.store(0);
            callCount.store(0);
            maxCycles.store(0);
            minCycles.store(UINT64_MAX);
        }

    private:
        static double cyclesToMicroseconds(uint64_t cycles) {
            // Approximate conversion - would need calibration for accuracy
            // This assumes a 3GHz CPU - should be calibrated for actual system
            return static_cast<double>(cycles) / 3000.0;
        }
    };

    class RealtimeTimer {
    public:
        RealtimeTimer(RealtimeStats& stats) : stats_(stats) {
#ifdef __x86_64__
            start_ = __builtin_ia32_rdtsc(); // x86-specific cycle counter
#else
            // Fallback for non-x86 architectures
            auto now = std::chrono::high_resolution_clock::now();
            start_ = static_cast<uint64_t>(now.time_since_epoch().count());
#endif
        }

        ~RealtimeTimer() {
#ifdef __x86_64__
            uint64_t end = __builtin_ia32_rdtsc();
#else
            auto now = std::chrono::high_resolution_clock::now();
            uint64_t end = static_cast<uint64_t>(now.time_since_epoch().count());
#endif
            uint64_t cycles = end - start_;

            stats_.totalCycles.fetch_add(cycles, std::memory_order_relaxed);
            stats_.callCount.fetch_add(1, std::memory_order_relaxed);

            // Update max cycles
            uint64_t currentMax = stats_.maxCycles.load(std::memory_order_relaxed);
            while (cycles > currentMax &&
                   !stats_.maxCycles.compare_exchange_weak(currentMax, cycles,
                                                          std::memory_order_relaxed)) {
                // Retry if another thread updated maxCycles
            }

            // Update min cycles
            uint64_t currentMin = stats_.minCycles.load(std::memory_order_relaxed);
            while (cycles < currentMin &&
                   !stats_.minCycles.compare_exchange_weak(currentMin, cycles,
                                                          std::memory_order_relaxed)) {
                // Retry if another thread updated minCycles
            }
        }

    private:
        RealtimeStats& stats_;
        uint64_t start_;
    };

    RealtimeTimer createTimer(const std::string& name) {
        return RealtimeTimer(stats_[name]);
    }

    void printStats() const {
        std::cout << "\n=== Realtime Profile Stats ===\n";
        std::cout << std::left << std::setw(20) << "Function"
                  << std::setw(12) << "Calls"
                  << std::setw(12) << "Avg (μs)"
                  << std::setw(12) << "Min (μs)"
                  << std::setw(12) << "Max (μs)"
                  << "\n";
        std::cout << std::string(68, '-') << "\n";

        for (const auto& [name, stats] : stats_) {
            std::cout << std::left << std::setw(20) << name
                      << std::setw(12) << stats.callCount.load()
                      << std::setw(12) << std::fixed << std::setprecision(2)
                      << stats.getAverageMicroseconds()
                      << std::setw(12) << stats.getMinMicroseconds()
                      << std::setw(12) << stats.getMaxMicroseconds()
                      << "\n";
        }
        std::cout << std::string(68, '=') << "\n";
    }

    void reset() {
        for (auto& [name, stats] : stats_) {
            stats.reset();
        }
    }

    RealtimeStats getStats(const std::string& name) const {
        auto it = stats_.find(name);
        return it != stats_.end() ? it->second : RealtimeStats{};
    }

private:
    std::map<std::string, RealtimeStats> stats_;
};

// Audio-specific performance metrics
class AudioPerformanceMonitor {
public:
    struct AudioMetrics {
        double cpuUsagePercent = 0.0;
        double bufferUnderruns = 0.0;
        double averageLatency = 0.0;
        double peakLatency = 0.0;
        size_t droppedSamples = 0;

        void reset() {
            cpuUsagePercent = 0.0;
            bufferUnderruns = 0.0;
            averageLatency = 0.0;
            peakLatency = 0.0;
            droppedSamples = 0;
        }
    };

    void startAudioBlock(size_t bufferSize, double sampleRate) {
        blockStart_ = std::chrono::high_resolution_clock::now();
        currentBufferSize_ = bufferSize;
        currentSampleRate_ = sampleRate;

        // Calculate expected processing time
        expectedProcessingTime_ = (bufferSize / sampleRate) * 1000000.0; // microseconds
    }

    void endAudioBlock() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - blockStart_);
        double processingTime = duration.count();

        // Update metrics
        metrics_.averageLatency = (metrics_.averageLatency * blockCount_ + processingTime) / (blockCount_ + 1);
        metrics_.peakLatency = std::max(metrics_.peakLatency, processingTime);

        // Calculate CPU usage as percentage of available time
        metrics_.cpuUsagePercent = (processingTime / expectedProcessingTime_) * 100.0;

        // Check for potential underruns
        if (processingTime > expectedProcessingTime_ * 0.8) { // 80% threshold
            metrics_.bufferUnderruns++;
        }

        blockCount_++;

        // Keep running average over last 100 blocks
        if (blockCount_ > 100) {
            blockCount_ = 100;
        }
    }

    void reportDroppedSamples(size_t count) {
        metrics_.droppedSamples += count;
    }

    const AudioMetrics& getMetrics() const {
        return metrics_;
    }

    void printAudioReport() const {
        std::cout << "\n=== Audio Performance Report ===\n";
        std::cout << "CPU Usage: " << std::fixed << std::setprecision(2)
                  << metrics_.cpuUsagePercent << "%\n";
        std::cout << "Average Latency: " << metrics_.averageLatency << " μs\n";
        std::cout << "Peak Latency: " << metrics_.peakLatency << " μs\n";
        std::cout << "Buffer Underruns: " << metrics_.bufferUnderruns << "\n";
        std::cout << "Dropped Samples: " << metrics_.droppedSamples << "\n";
        std::cout << "Blocks Processed: " << blockCount_ << "\n";
        std::cout << std::string(35, '=') << "\n";
    }

    void reset() {
        metrics_.reset();
        blockCount_ = 0;
    }

private:
    AudioMetrics metrics_;
    std::chrono::high_resolution_clock::time_point blockStart_;
    size_t currentBufferSize_ = 0;
    double currentSampleRate_ = 0.0;
    double expectedProcessingTime_ = 0.0;
    size_t blockCount_ = 0;
};

// Memory usage profiler
class MemoryProfiler {
public:
    struct MemoryStats {
        size_t currentUsage = 0;
        size_t peakUsage = 0;
        size_t totalAllocations = 0;
        size_t totalDeallocations = 0;
        size_t activeAllocations = 0;

        void reset() {
            currentUsage = 0;
            peakUsage = 0;
            totalAllocations = 0;
            totalDeallocations = 0;
            activeAllocations = 0;
        }
    };

    void recordAllocation(size_t size) {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.currentUsage += size;
        stats_.peakUsage = std::max(stats_.peakUsage, stats_.currentUsage);
        stats_.totalAllocations++;
        stats_.activeAllocations++;
    }

    void recordDeallocation(size_t size) {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.currentUsage = (size > stats_.currentUsage) ? 0 : stats_.currentUsage - size;
        stats_.totalDeallocations++;
        if (stats_.activeAllocations > 0) {
            stats_.activeAllocations--;
        }
    }

    MemoryStats getStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return stats_;
    }

    void printMemoryReport() const {
        auto stats = getStats();
        std::cout << "\n=== Memory Usage Report ===\n";
        std::cout << "Current Usage: " << formatBytes(stats.currentUsage) << "\n";
        std::cout << "Peak Usage: " << formatBytes(stats.peakUsage) << "\n";
        std::cout << "Total Allocations: " << stats.totalAllocations << "\n";
        std::cout << "Total Deallocations: " << stats.totalDeallocations << "\n";
        std::cout << "Active Allocations: " << stats.activeAllocations << "\n";
        std::cout << std::string(30, '=') << "\n";
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.reset();
    }

private:
    MemoryStats stats_;
    mutable std::mutex mutex_;

    std::string formatBytes(size_t bytes) const {
        const char* units[] = {"B", "KB", "MB", "GB"};
        double size = static_cast<double>(bytes);
        int unit = 0;

        while (size >= 1024.0 && unit < 3) {
            size /= 1024.0;
            unit++;
        }

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << size << " " << units[unit];
        return oss.str();
    }
};

// Global instances
inline RealtimeProfiler& getRealtimeProfiler() {
    static RealtimeProfiler profiler;
    return profiler;
}

inline AudioPerformanceMonitor& getAudioMonitor() {
    static AudioPerformanceMonitor monitor;
    return monitor;
}

inline MemoryProfiler& getMemoryProfiler() {
    static MemoryProfiler profiler;
    return profiler;
}

// Convenience macros
#define PROFILE_REALTIME(name) \
    auto _rt_timer = getRealtimeProfiler().createTimer(name)

#define PROFILE_AUDIO_BLOCK(bufferSize, sampleRate) \
    getAudioMonitor().startAudioBlock(bufferSize, sampleRate); \
    auto _audio_guard = [](){ getAudioMonitor().endAudioBlock(); }; \
    std::unique_ptr<void, decltype(_audio_guard)> _audio_timer(nullptr, _audio_guard)

} // namespace ModernAudio::Profiling

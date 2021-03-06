// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

#ifndef PBRT_UTIL_PROGRESSREPORTER_H
#define PBRT_UTIL_PROGRESSREPORTER_H

#include <pbrt/pbrt.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>

#ifdef PBRT_BUILD_GPU_RENDERER
#include <cuda_runtime.h>
#include <pbrt/util/pstd.h>
#include <vector>
#endif

namespace pbrt {

// Timer Definition
class Timer {
  public:
    Timer() { start = clock::now(); }
    double ElapsedSeconds() const {
        clock::time_point now = clock::now();
        int64_t elapseduS =
            std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
        return elapseduS / 1000000.;
    }

    std::string ToString() const;

  private:
    using clock = std::chrono::steady_clock;
    clock::time_point start;
};

// ProgressReporter Definition
class ProgressReporter {
  public:
    // ProgressReporter Public Methods
    ProgressReporter() : quiet(true) {}
    ProgressReporter(int64_t totalWork, const std::string &title, bool quiet,
                     bool gpu = false);
    ~ProgressReporter();

    void Update(int64_t num = 1) {
#ifdef PBRT_BUILD_GPU_RENDERER
        if (gpuEvents.size() > 0) {
            CHECK_LE(gpuEventsLaunchedOffset + num, gpuEvents.size());
            while (num-- > 0) {
                CHECK_EQ(cudaEventRecord(gpuEvents[gpuEventsLaunchedOffset]),
                         cudaSuccess);
                ++gpuEventsLaunchedOffset;
            }
            return;
        }
#endif
        if (num == 0 || quiet)
            return;
        workDone += num;
    }
    double ElapsedSeconds() const { return timer.ElapsedSeconds(); }
    void Done();

    std::string ToString() const;

  private:
    // ProgressReporter Private Methods
    void launchThread();
    void printBar();

    // ProgressReporter Private Data
    int64_t totalWork;
    std::string title;
    bool quiet;
    Timer timer;
    std::atomic<int64_t> workDone;
    std::atomic<bool> exitThread;
    std::thread updateThread;

#ifdef PBRT_BUILD_GPU_RENDERER
    std::vector<cudaEvent_t> gpuEvents;
    std::atomic<int> gpuEventsLaunchedOffset;
    int gpuEventsFinishedOffset;
#endif
};

}  // namespace pbrt

#endif  // PBRT_UTIL_PROGRESSREPORTER_H

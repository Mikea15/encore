#include "profiler.h"

// Define the thread_local static member
thread_local ThreadSafeProfiler::ThreadData* ThreadSafeProfiler::tl_threadData = nullptr;

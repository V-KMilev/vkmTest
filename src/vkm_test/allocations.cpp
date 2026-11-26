#include "vkm_test/scratch.h"

#include <atomic>
#include <cstdlib>
#include <new>

#if defined(_WIN32)
    #include <malloc.h>
#endif

// A sanitizer brings an operator new and delete of its own, to watch every block: the counting replacements stay out of its way, and allocations() skips.
#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_THREAD__)
    #define VKM_TEST_SANITIZED
#elif defined(__has_feature)
    #if __has_feature(address_sanitizer) || __has_feature(thread_sanitizer) || __has_feature(memory_sanitizer)
        #define VKM_TEST_SANITIZED
    #endif
#endif

#if !defined(VKM_TEST_SANITIZED)

namespace {

std::atomic<uint64_t> g_allocations{0};

/// A block of @p size bytes, counted for allocations(); null when there is no memory for it.
void* counted(std::size_t size) {
    g_allocations.fetch_add(1, std::memory_order_relaxed);
    return std::malloc(size ? size : 1);
}

/// A block of @p size bytes on an @p align boundary, counted; null when there is no memory for it.
void* countedAligned(std::size_t size, std::align_val_t align) {
    g_allocations.fetch_add(1, std::memory_order_relaxed);
    const std::size_t alignment = static_cast<std::size_t>(align);
#if defined(_WIN32)
    return _aligned_malloc(size ? size : 1, alignment);
#else
    // aligned_alloc takes a whole number of alignments.
    return std::aligned_alloc(alignment, (size / alignment + 1) * alignment);
#endif
}

/// Returns a block countedAligned gave.
void releaseAligned(void* block) {
#if defined(_WIN32)
    _aligned_free(block);
#else
    std::free(block);
#endif
}

} // namespace

#endif

namespace Vkm::Test {

uint64_t allocations() {
#if defined(VKM_TEST_SANITIZED)
    skip("a sanitizer's own operator new is not counted");
#else
    return g_allocations.load(std::memory_order_relaxed);
#endif
}

} // namespace Vkm::Test

#if !defined(VKM_TEST_SANITIZED)

// GCC 11 and up, inlining a delete here or in a test, sees std::free take a block from operator new; these replacements are the pair that makes it right, so the warning is off to the end of the file.
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 11
    #pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif

// Every form, counted for allocations(): the suite's and the static libraries' calls, and a shared library's where the platform resolves them here, which a Windows DLL's, kept with its own runtime, are not.
void* operator new(std::size_t size) {
    if (void* block = counted(size)) return block;
    throw std::bad_alloc();
}
void* operator new(std::size_t size, std::align_val_t align) {
    if (void* block = countedAligned(size, align)) return block;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return operator new(size); }
void* operator new[](std::size_t size, std::align_val_t align) { return operator new(size, align); }
void* operator new(std::size_t size, const std::nothrow_t&) noexcept { return counted(size); }
void* operator new[](std::size_t size, const std::nothrow_t&) noexcept { return counted(size); }
void* operator new(std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept {
    return countedAligned(size, align);
}
void* operator new[](std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept {
    return countedAligned(size, align);
}
void operator delete(void* block) noexcept { std::free(block); }
void operator delete[](void* block) noexcept { std::free(block); }
void operator delete(void* block, std::size_t) noexcept { std::free(block); }
void operator delete[](void* block, std::size_t) noexcept { std::free(block); }
void operator delete(void* block, const std::nothrow_t&) noexcept { std::free(block); }
void operator delete[](void* block, const std::nothrow_t&) noexcept { std::free(block); }
void operator delete(void* block, std::align_val_t) noexcept { releaseAligned(block); }
void operator delete[](void* block, std::align_val_t) noexcept { releaseAligned(block); }
void operator delete(void* block, std::size_t, std::align_val_t) noexcept { releaseAligned(block); }
void operator delete[](void* block, std::size_t, std::align_val_t) noexcept { releaseAligned(block); }
void operator delete(void* block, std::align_val_t, const std::nothrow_t&) noexcept { releaseAligned(block); }
void operator delete[](void* block, std::align_val_t, const std::nothrow_t&) noexcept {
    releaseAligned(block);
}

#endif

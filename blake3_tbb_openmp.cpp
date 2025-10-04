#include <cstddef>
#include <cstdint>

#if defined(_OPENMP)
    #include <omp.h>
#endif

#if defined(BLAKE3_USE_TBB)
    #include <Z:/work/oneapi-tbb-2022.1.0/include/oneapi/tbb/parallel_invoke.h>
    #include <Z:/work/oneapi-tbb-2022.1.0/include/oneapi/tbb/task_arena.h>
#endif

#include ".\BLAKE3\c\blake3_impl.h"

#if defined(BLAKE3_USE_TBB)
static_assert(TBB_USE_EXCEPTIONS == 0,
              "This file should be compiled with C++ exceptions disabled.");
#endif

extern "C" int mt_selector = 0;

#if defined(BLAKE3_USE_TBB)
    oneapi::tbb::task_arena light_arena2(2);
    oneapi::tbb::task_arena light_arena4(4);
#endif

    extern "C" int tbb_max_concurrency()
    {
        int max_concurrency = 1;

#if defined(BLAKE3_USE_TBB)
        if (mt_selector == 1)
        {
            oneapi::tbb::parallel_invoke(
                [&]() {
                    max_concurrency = oneapi::tbb::this_task_arena::max_concurrency();
                },
                [&]() {
                    //max_concurrency = oneapi::tbb::this_task_arena::max_concurrency();
                }
            );
        }
        else if (mt_selector == 2)
        {
            light_arena2.execute([&]() {
                oneapi::tbb::parallel_invoke(
                    [&]() {
                        max_concurrency = oneapi::tbb::this_task_arena::max_concurrency();
                    },
                    [&]() {
                        //max_concurrency = oneapi::tbb::this_task_arena::max_concurrency();
                    }
                );
            });
        }
        else if (mt_selector == 3)
        {
            light_arena4.execute([&]() {
                oneapi::tbb::parallel_invoke(
                    [&]() {
                        max_concurrency = oneapi::tbb::this_task_arena::max_concurrency();
                    },
                    [&]() {
                        //max_concurrency = oneapi::tbb::this_task_arena::max_concurrency();
                    }
                );
                });
        }
        else if (mt_selector == 4)
        {
            max_concurrency = 2;
        }
#endif
        return max_concurrency;
    }

extern "C" void blake3_compress_subtree_wide_join_tbb(
    // shared params
    const uint32_t key[8], uint8_t flags, bool use_tbb,
    // left-hand side params
    const uint8_t* l_input, size_t l_input_len, uint64_t l_chunk_counter,
    uint8_t* l_cvs, size_t* l_n,
    // right-hand side params
    const uint8_t* r_input, size_t r_input_len, uint64_t r_chunk_counter,
    uint8_t* r_cvs, size_t* r_n) noexcept {
    if (!use_tbb) {
        *l_n = blake3_compress_subtree_wide(l_input, l_input_len, key,
            l_chunk_counter, flags, l_cvs, use_tbb);
        *r_n = blake3_compress_subtree_wide(r_input, r_input_len, key,
            r_chunk_counter, flags, r_cvs, use_tbb);
        return;
    }

#if defined(BLAKE3_USE_TBB)
    if (mt_selector == 1)
    {
        oneapi::tbb::parallel_invoke(
            [=]() {
                *l_n = blake3_compress_subtree_wide(
                    l_input, l_input_len, key, l_chunk_counter, flags, l_cvs, use_tbb);
            },
            [=]() {
                *r_n = blake3_compress_subtree_wide(
                    r_input, r_input_len, key, r_chunk_counter, flags, r_cvs, use_tbb);
            });
    }
    else if (mt_selector==2)
    {
        light_arena2.execute([&]() {
            oneapi::tbb::parallel_invoke(
                [=]() {
                    *l_n = blake3_compress_subtree_wide(
                        l_input, l_input_len, key, l_chunk_counter, flags, l_cvs, use_tbb);
                },
                [=]() {
                    *r_n = blake3_compress_subtree_wide(
                        r_input, r_input_len, key, r_chunk_counter, flags, r_cvs, use_tbb);
                });
            });
    }
    else if (mt_selector==3)
    {
        light_arena4.execute([&]() {
            oneapi::tbb::parallel_invoke(
                [=]() {
                    *l_n = blake3_compress_subtree_wide(
                        l_input, l_input_len, key, l_chunk_counter, flags, l_cvs, use_tbb);
                },
                [=]() {
                    *r_n = blake3_compress_subtree_wide(
                        r_input, r_input_len, key, r_chunk_counter, flags, r_cvs, use_tbb);
                });
            });
    }
#if defined(_OPENMP)
    else if (mt_selector==4)
    {
        // move to main() function
        //omp_set_num_threads(2)
        //omp_set_max_active_levels(2);
        //omp_set_max_active_levels(3);

#pragma omp parallel sections //num_threads(2)
        {
#pragma omp section
            {
                *l_n = blake3_compress_subtree_wide(
                    l_input, l_input_len, key, l_chunk_counter, flags, l_cvs, use_tbb);
            }

#pragma omp section
            {
                *r_n = blake3_compress_subtree_wide(
                    r_input, r_input_len, key, r_chunk_counter, flags, r_cvs, use_tbb);
            }
        }
    }
#endif
    else
    {
        *l_n = blake3_compress_subtree_wide(l_input, l_input_len, key,
            l_chunk_counter, flags, l_cvs, use_tbb);
        *r_n = blake3_compress_subtree_wide(r_input, r_input_len, key,
            r_chunk_counter, flags, r_cvs, use_tbb);
    }
#endif
}

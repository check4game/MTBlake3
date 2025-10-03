Intel i7-10700K 8C/16, oneapi-tbb-2022.1.0
v1.1, fileSize: 14461091840, sizeof(buf): 2097152, D:\temp\test3mix.1CD

```
CPU Usage: SGL,  1 Threads | 10%
CPU Usage: TBB, 16 Threads | 100%
CPU Usage: TBB,  2 Threads | 17%
CPU Usage: TBB,  4 Threads | 32%
CPU Usage: OMP,  2 Threads | 32%

SGL 1 Threads, no TBB or OMP, blake3_hasher_update
 
TBB 16 Threads by default

        oneapi::tbb::parallel_invoke(
            [=]() {
                *l_n = blake3_compress_subtree_wide(
                    l_input, l_input_len, key, l_chunk_counter, flags, l_cvs, use_tbb);
            },
            [=]() {
                *r_n = blake3_compress_subtree_wide(
                    r_input, r_input_len, key, r_chunk_counter, flags, r_cvs, use_tbb);
            });

TBB 2 Threads

        oneapi::tbb::task_arena light_arena2(2);

        light_arena.execute([&]() {
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

TBB 4 Threads

        oneapi::tbb::task_arena light_arena4(4);

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

OMP 2 Threads

        omp_set_max_active_levels(2);

        #pragma omp parallel sections num_threads(2)
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
```
https://github.com/check4game/MTBlake3/blob/main/i7-10700K.results.v2.txt

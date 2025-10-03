Intel i7-10700K 8C/16, oneapi-tbb-2022.1.0

https://github.com/check4game/MTBlake3/blob/main/i7-10700K.results.txt

```
CPU Usage: SGL,  1 Threads | 8%
CPU Usage: TBB, 16 Threads | 100%
CPU Usage: TBB,  2 Threads | 16%
CPU Usage: OMP,  2 Threads | 16%

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

        oneapi::tbb::task_arena light_arena(2);

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

OMP 2 Threads
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

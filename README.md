Intel i7-10700K 8C/16, oneapi-tbb-2022.1.0

https://github.com/check4game/MTBlake3/blob/main/i7-10700K.results.v.1.2.txt

```
v1.2, fileSize: 14461091840, readSize: 2097152, D:\temp\test3mix.1CD
SGL,    1T, Time:  5.027 sec, CPU:   6.48%, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23
TBB,   16T, Time:  3.087 sec, CPU:  99.62%, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23
TBB,    2T, Time:  3.854 sec, CPU:  12.83%, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23
TBB,    4T, Time:  3.077 sec, CPU:  25.38%, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23
OMP, 2L|2T, Time:  3.064 sec, CPU:  25.19%, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23
OMP, 3L|2T, Time:  2.952 sec, CPU:  50.34%, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23

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

        omp_set_num_threads(2); omp_set_max_active_levels(2); // omp_set_max_active_levels(3);

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
```

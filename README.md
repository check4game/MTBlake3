Intel i7-10700K 8C/16, oneapi-tbb-2022.1.0

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
```
Z:\work\MZ\MTBlake3\x64\Release>MTBlake3.exe D:\temp\test3mix.1CD

SGL,  1 Threads, Time: 5.074000 sec, Size: 14461091840, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23
SGL,  1 Threads, Time: 5.073000 sec, Size: 14461091840, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23
SGL,  1 Threads, Time: 5.078000 sec, Size: 14461091840, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23

TBB, 16 Threads, Time: 3.937000 sec, Size: 14461091840, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23
TBB, 16 Threads, Time: 3.963000 sec, Size: 14461091840, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23
TBB, 16 Threads, Time: 3.949000 sec, Size: 14461091840, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23

TBB,  2 Threads, Time: 4.102000 sec, Size: 14461091840, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23
TBB,  2 Threads, Time: 4.092000 sec, Size: 14461091840, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23
TBB,  2 Threads, Time: 4.100000 sec, Size: 14461091840, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23

OMP,  2 Threads, Time: 4.023000 sec, Size: 14461091840, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23
OMP,  2 Threads, Time: 4.015000 sec, Size: 14461091840, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23
OMP,  2 Threads, Time: 4.022000 sec, Size: 14461091840, Hash: df27698036f83e2bdbab0beff6b8351e562bdaceeec14f09554344f08b723f23
```

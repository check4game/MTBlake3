#include ".\BLAKE3\c\blake3.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>

#include <windows.h>

extern int mt_selector;

extern int tbb_max_concurrency();

static unsigned char buf[2 * 1024 * 1024];

static int TEST(char * file, int mt)
{
#if !defined(BLAKE3_USE_TBB)
	mt = 0;
#endif

	if (mt < 0 || mt > 4) mt = 0;

	mt_selector = mt;

	int fn = _open(file, _O_RDONLY | _O_BINARY);

	if (fn < 0)
	{
		fprintf(stderr, "open failed: %s\n", strerror(errno)); return 0;
	}

	// Initialize the hasher.
	blake3_hasher hasher;
	blake3_hasher_init(&hasher);

	clock_t start = clock();

	while (1)
	{
		size_t n = read(fn, buf, sizeof(buf));

		if (n > 0)
		{
#if !defined(BLAKE3_USE_TBB)
			blake3_hasher_update(&hasher, buf, n);
#else
			if (mt)
				blake3_hasher_update_tbb(&hasher, buf, n);
			else
				blake3_hasher_update(&hasher, buf, n);				
#endif
		}
		else if (n == 0)
		{
			break; // end of file
		}
		else
		{
			fprintf(stderr, "read failed: %s\n", strerror(errno));
			return 1;
		}
	}

	_close(fn);

	// Finalize the hash. BLAKE3_OUT_LEN is the default output length, 32 bytes.
	uint8_t output[BLAKE3_OUT_LEN];
	blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);

	FILETIME idleTime2, kernelTime2, userTime2;

	if (!GetSystemTimes(&idleTime2, &kernelTime2, &userTime2))
	{
		fprintf(stderr, "GetSystemTimes failed: %s\n", strerror(errno)); return errno;
	}

	clock_t end = clock();

	double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;

	if (!mt)
	{
		printf("SGL");
	}
	else if (mt == 1)
	{
		printf("TBB");
	}
	else if (mt == 2)
	{
		printf("TBB");
	}
	else if (mt == 3)
	{
		printf("TBB");
	}
	else if (mt == 4)
	{
		printf("OMP");
	}

	printf(", %2d Threads, ", tbb_max_concurrency());

	printf("Time: %.3f sec, Hash: ", elapsed);

	// Print the hash as hexadecimal.
	for (size_t i = 0; i < BLAKE3_OUT_LEN; i++) {
		printf("%02x", output[i]);
	}
	printf("\n"); return 0;

	Sleep(1000);
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("%s file.ext\n", argv[0]); return 0;
	}

	int fn = _open(argv[1], _O_RDONLY | _O_BINARY);

	if (fn < 0)
	{
		fprintf(stderr, "open failed: %s\n", strerror(errno)); return errno;
	}

	long long fileSize = _filelengthi64(fn);

	printf("v1.1, fileSize: %lld, sizeof(buf): %lld, %s\n", fileSize, sizeof(buf), argv[1]);

	_close(fn);

	for (int i = 0; i < 5; i++)
	{
		TEST(argv[1], i);
		TEST(argv[1], i);
		TEST(argv[1], i);
		printf("\n");
	}
}

#include ".\BLAKE3\c\blake3.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>

#include <windows.h>

#include <omp.h>

#include <pdh.h>
#include <pdhmsg.h>

#pragma comment(lib, "pdh.lib")

typedef struct {
	PDH_HQUERY query;
	PDH_HCOUNTER cpuCounter;
	DWORD processId;
} SELF_MONITOR;

static SELF_MONITOR monitor;

static const char* GetPdhErrorString(PDH_STATUS status) {
	switch (status) {
	case ERROR_SUCCESS: return "Success";
	case PDH_CSTATUS_NO_OBJECT: return "Object not found";
	case PDH_CSTATUS_NO_COUNTER: return "Counter not found";
	case PDH_CSTATUS_NO_INSTANCE: return "Instance not found";
	case PDH_INVALID_DATA: return "Invalid data";
	case PDH_NO_DATA: return "No data available";
	case PDH_MORE_DATA: return "More data available";
	default: return "Unknown error";
	}
}

static int InitializeSelfMonitor(SELF_MONITOR* monitor) {
	PDH_STATUS status;

	// Получаем ID текущего процесса
	monitor->processId = GetCurrentProcessId();

	status = PdhOpenQuery(NULL, 0, &monitor->query);
	if (status != ERROR_SUCCESS)
	{
		printf("PdhOpenQuery: %s\n", GetPdhErrorString(status));
		return 0;
	}

	
#if 0
	WCHAR counterPath[1024] = { 0 };
	swprintf(counterPath, 1024, L"\\Process(%d)\\% Processor Time", monitor->processId);
	status = PdhAddCounter(monitor->query, counterPath, 0, &monitor->cpuCounter);
#endif

	status = PdhAddCounter(monitor->query, L"\\Processor(_Total)\\% Processor Time", 0, &monitor->cpuCounter);
	//status = PdhAddCounter(monitor->query, L"\\Process(MTBlake3)\\% Processor Time", 0, &monitor->cpuCounter);

	if (status != ERROR_SUCCESS)
	{
		printf("PdhAddCounter: 0x%X, %s\n", status, GetPdhErrorString(status));
		PdhCloseQuery(monitor->query);
		return 0;
	}

	// Инициализируем сбор данных
	PdhCollectQueryData(monitor->query);

	return 1;
}

static double GetSelfCPUUsage(SELF_MONITOR* monitor, int sleep)
{
	PDH_STATUS status = ERROR_SUCCESS;

	if (sleep > 0)
	{
		Sleep(sleep);
	}

	status = PdhCollectQueryData(monitor->query);

	if (status != ERROR_SUCCESS)
	{
		printf("PdhCollectQueryData: %s\n", GetPdhErrorString(status));
		return -1.0;
	}
	
	PDH_FMT_COUNTERVALUE value;
	status = PdhGetFormattedCounterValue(monitor->cpuCounter, PDH_FMT_DOUBLE, NULL, &value);

	if (status == ERROR_SUCCESS)
	{
		return value.doubleValue;
	}

	printf("PdhGetFormattedCounterValue: %s\n", GetPdhErrorString(status));
	return -1.0;
}

static void CloseSelfMonitor(SELF_MONITOR* monitor)
{
	if (monitor->query)
	{
		PdhCloseQuery(monitor->query);
	}
}

//------------------------------------------

extern int mt_selector;

extern int tbb_max_concurrency();

static unsigned char buf[64 * 1024 * 1024];

static int TEST(char * file, int mt, int readSizeKB)
{
#if !defined(BLAKE3_USE_TBB)
	mt = 0;
#endif

	if (readSizeKB < 128 || readSizeKB > (sizeof(buf) / 1024))
	{
		readSizeKB = 2048;
	}

	int readSize = readSizeKB * 1024;

	if (mt < 0 || mt > 4) mt = 0;

	mt_selector = mt;

	int fn = _open(file, _O_RDONLY | _O_BINARY);

	if (fn < 0)
	{
		fprintf(stderr, "open failed: %s\n", strerror(errno)); return 0;
	}

	double cpu_before = GetSelfCPUUsage(&monitor, 3000);

	// Initialize the hasher.
	blake3_hasher hasher;
	blake3_hasher_init(&hasher);

	clock_t start = clock();

	while (1)
	{
		size_t n = read(fn, buf, readSize);

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

	double cpu_after = GetSelfCPUUsage(&monitor, 0);

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

	if (mt < 4)
	{
		printf(",   %2dT, ", tbb_max_concurrency());
	}
	else
	{
		printf(", %dL|%dT, ", omp_get_max_active_levels(), omp_get_max_threads());
	}

	printf("Time: %6.3f sec, CPU: %6.2f%%, Hash: ", elapsed, cpu_after);

	// Print the hash as hexadecimal.
	for (size_t i = 0; i < BLAKE3_OUT_LEN; i++) {
		printf("%02x", output[i]);
	}
	printf("\n"); return 0;
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

	_close(fn);

	if (!InitializeSelfMonitor(&monitor)) {
		return -1;
	}

	for (int readSizeKB = 1024; readSizeKB <= (sizeof(buf) / 1024); readSizeKB *= 2)
	{
		printf("\nv1.2, fileSize: %lld, readSize: %lld, %s\n", fileSize, readSizeKB * 1024LL, argv[1]);

		for (int mt = 0; mt < 4; mt++)
		{
			TEST(argv[1], mt, readSizeKB);
		}

		int omp_num_threads = 2;

		for (int max_active_levels = 2; max_active_levels < 4; max_active_levels++)
		{
			omp_set_num_threads(omp_num_threads); omp_set_max_active_levels(max_active_levels);

			TEST(argv[1], 4, readSizeKB);
		}
	}

	CloseSelfMonitor(&monitor);

	return 0;
}


//-------------------------------------------------------------------------------------------------
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

namespace system_util{

static inline int memReadStat(int field)
{
	char  name[256];
	pid_t pid = getpid();
	int   value;

	sprintf(name, "/proc/%d/statm", pid);
	FILE* in = fopen(name, "rb");
	if (in == NULL) return 0;
	for (; field >= 0; field--)
		if (fscanf(in, "%d", &value) != 1)
	printf("ERROR! Failed to parse memory statistics from \"/proc\".\n"), exit(1);
	fclose(in);
	return value;
}

static inline double cpuTime(void) { return (double)clock() / CLOCKS_PER_SEC; }

static inline int memReadPeak(void){
	char  name[256];
	pid_t pid = getpid();
	sprintf(name, "/proc/%d/status", pid);
	FILE* in = fopen(name, "rb");
	if (in == NULL) return 0;
			    
	int peak_kb = 0;
	while (!feof(in) && fscanf(in, "VmPeak: %d kB", &peak_kb) != 1)
		while (!feof(in) && fgetc(in) != '\n')
			;
	fclose(in);
	return peak_kb;
}

static inline double memUsed() { return (double)memReadStat(0) * (double)getpagesize() / (1024*1024); }

static inline double memUsedPeak() { 
	double peak = memReadPeak() / 1024;
	return peak == 0 ? memUsed() : peak; 
}
};

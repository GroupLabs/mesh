#include <stdio.h>
#include <string.h>

const int SKIPCHAR = 2;

//More efficient to run this than run lscpu?
void show_cpu_info(char* buffer, size_t buffer_size) {
	printf("%lu", buffer_size);
	FILE * file = fopen("/proc/cpuinfo", "r");
	if (file == NULL) {
		perror("fopen");
		return;
	}

	while (fgets(buffer, buffer_size, file) != NULL) {
		if (strncmp(buffer, "model name", 10) == 0) {
			char* brand_string = strchr(buffer, ':') + SKIPCHAR;
			snprintf(buffer, buffer_size, "%s", brand_string);
			break;
		}
	}

	fclose(file);
	printf("%s", buffer);
}

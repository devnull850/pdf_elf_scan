#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEN 256

int main(void) {
	FILE *fd;
	char *buf, *p, filename[LEN+1];
	long len;

	fgets(filename, LEN, stdin);
	if ((p = strchr(filename, 0xa))) {
		*p = 0;
	}

	if (!(fd = fopen(filename, "rb"))) {
		fprintf(stderr, "error opening [%s]\n", filename);
		exit(EXIT_FAILURE);
	}

	fseek(fd, 0, SEEK_END);
	len = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	if (!(buf = malloc(len))) {
		fprintf(stderr, "error allocating memory [buf]\n");
		exit(EXIT_FAILURE);
	}

	fread(buf, 1, len, fd);

	if (fclose(fd) == EOF) {
		fprintf(stderr, "error closing [%s]\n", filename);
		exit(EXIT_FAILURE);
	}

	puts(filename);

	p = buf;

	while ((p = memmem(p, len - (p - buf), "ELF", 3))) {
		printf("0x%0lx\n", p - buf);
		p += 3;
	}

	free(buf);

	return EXIT_SUCCESS;
}

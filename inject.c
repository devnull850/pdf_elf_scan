#include <stdio.h>
#include <stdlib.h>

int main(void) {
	FILE *fd;
	char *blob, *pdf;
	long blobLength, pdfLength;

	if (!(fd = fopen("test.pdf", "rb"))) {
		fprintf(stderr, "error opening [test.pdf]\n");
		exit(EXIT_FAILURE);
	}

	fseek(fd, 0, SEEK_END);
	pdfLength = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	if (!(pdf = malloc(pdfLength))) {
		fprintf(stderr, "error allocating memory [pdf]\n");
		exit(EXIT_FAILURE);
	}

	fread(pdf, 1, pdfLength, fd);

	if (fclose(fd) == EOF) {
		fprintf(stderr, "error closing [test.pdf]\n");
		exit(EXIT_FAILURE);
	}

	if (!(fd = fopen("a.out", "rb"))) {
		fprintf(stderr, "error opening [a.out]\n");
		exit(EXIT_FAILURE);
	}

	fseek(fd, 0, SEEK_END);
	blobLength = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	if (!(blob = malloc(blobLength))) {
		fprintf(stderr, "error allocating memory [blob]\n");
		exit(EXIT_FAILURE);
	}

	fread(blob, 1, blobLength, fd);

	if (fclose(fd) == EOF) {
		fprintf(stderr, "error closing [a.out]\n");
		exit(EXIT_FAILURE);
	}

	for (size_t i = 0; i < blobLength; ++i) {
		pdf[i+0xf800] = blob[i];
	}

	if (!(fd = fopen("test1.pdf", "wb"))) {
		fprintf(stderr, "error opening [test1.pdf]\n");
		exit(EXIT_FAILURE);
	}

	fwrite(pdf, 1, pdfLength, fd);

	if (fclose(fd) == EOF) {
		fprintf(stderr, "error closing [test1.pdf]\n");
		exit(EXIT_FAILURE);
	}

	free(blob);
	free(pdf);

	return EXIT_SUCCESS;
}

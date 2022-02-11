#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <capstone/capstone.h>

#define IN_LEN 256

int is_cs_cflow_group(uint8_t);
int is_cs_cflow_ins(cs_insn *);
int is_seen(uint64_t, size_t);

uint64_t seen[1024];

int main(void) {
	FILE *fd;
	char *s, *filename;
	long len, addr, entry;
	char in[IN_LEN + 1];
	csh handle;
	cs_insn *insn;
	uint8_t *buf, *p, *p1, b;
	size_t count, code_size;
	uint64_t target;
	size_t seen_index;
	int flag;

	fgets(in, IN_LEN, stdin);
	if ((filename = strchr(in, 0xa))) {
		*filename = 0;
	}

	if (!(filename = malloc(strlen(in)+1))) {
		fprintf(stderr, "error allocating memory\n");
		exit(EXIT_FAILURE);
	}

	strncpy(filename, in, strlen(in));

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

	free(filename);

	if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
		fprintf(stderr, "error opening capstone handle\n");
		exit(EXIT_FAILURE);
	}

	cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

	while (fgets(in, IN_LEN, stdin)) {
		if ((s = strchr(in, 0xa))) {
			*s = 0;
		} 

		addr = strtol(in, NULL, 16);
		--addr;				// 0x7f before "ELF"

		memcpy(&entry, buf + addr + 0x18, sizeof(long));
		printf("entry point: 0x%0lx\n\n", entry);

		p1 = (uint8_t *) (buf + addr);
		p = (uint8_t *) (buf + addr + entry);
		code_size = p - buf;

		seen_index = 0;
		flag = 1;

		while (flag) {
			count = cs_disasm(handle, p, code_size, 0, 0, &insn);

			if (!count) {
				break;
			}

			for (size_t i = 0; i < count; ++i) {
				printf("0x%0lx:\t%s\t\t%s\n", insn[i].address,
					insn[i].mnemonic, insn[i].op_str);

				if (is_cs_cflow_ins(insn+i)) {
					b = insn[i].bytes[1];

					if (((b>>7) & 1)) {
						b = 0 - b;
						target = (p - p1) +
							insn[i].address +
							insn[i].size;
						target -= b;
					}
					else {
						target = (p - p1) + b +
							insn[i].address +
							insn[i].size;
					}

					printf("jumping to 0x%0lx\n\n", target);

					if (is_seen(target, seen_index)) {
						flag = 0;
						break;
					}

					seen[seen_index++] = target;
					p = p1 + target;
					code_size = (p1 + target) - buf;

					break;
				}
			}

			cs_free(insn, count);
		}
	}

	cs_close(&handle);

	free(buf);

	return EXIT_SUCCESS;
}

int is_cs_cflow_group(uint8_t g) {
	return g == CS_GRP_JUMP || g == CS_GRP_CALL ||
		g == CS_GRP_RET || g == CS_GRP_IRET;
}

int is_cs_cflow_ins(cs_insn *ins) {
	for (size_t i = 0; i < ins->detail->groups_count; ++i) {
		if (is_cs_cflow_group(ins->detail->groups[i])) {
			return 1;
		}
	}

	return 0;
}

int is_seen(uint64_t a, size_t i) {
	for (size_t j = 0; j < i; ++j) {
		if (a == seen[j]) {
			return 1;
		}
	}

	return 0;
}

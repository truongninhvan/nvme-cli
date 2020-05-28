#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include <libnvme.h>
#include "nvme.h"
#include "plugin.h"
#include "argconfig.h"
#include "suffix.h"

#define CREATE_CMD
#include "transcend-nvme.h"

static const __u32 OP_BAD_BLOCK = 0xc2;
static const __u32 DW10_BAD_BLOCK = 0x400;
static const __u32 DW12_BAD_BLOCK = 0x5a;

static int getHealthValue(int argc, char **argv, struct command *cmd, struct plugin *plugin)
{
	struct nvme_smart_log smart_log;
	char *desc = "Get nvme health percentage.";
 	int result, fd;
	 
	OPT_ARGS(opts) = {
		OPT_END()
	};

	fd = parse_and_open(argc, argv, desc, opts);
	if (fd < 0) {
		printf("\nDevice not found \n");;
		return -1;
	}

	result = nvme_get_log_smart(fd, 0xffffffff, false, &smart_log);
	if (!result) {
		int percent_used = smart_log.percent_used;

		printf("Transcend NVME heath value: ");
		if (percent_used > 100 || percent_used < 0)
			printf("0%%\n");
		else
			printf("%d%%\n", 100 - percent_used);
	}

	return result;
}
 
static int getBadblock(int argc, char **argv, struct command *cmd, struct plugin *plugin)
{
	char *desc = "Get nvme bad block number.";
 	int result, fd;

	unsigned char data[1]={ };

	struct nvme_passthru_cmd nvmecmd = {
		.opcode = OP_BAD_BLOCK,
		.cdw10 = DW10_BAD_BLOCK,
		.cdw12 = DW12_BAD_BLOCK,
		.addr = (__u64)(uintptr_t)data,
		.data_len = 0x1,
	};

	OPT_ARGS(opts) = {
		OPT_END()
	};

	fd = parse_and_open(argc, argv, desc, opts);
	if (fd < 0) {
		printf("\nDevice not found \n");;
		return -1;
	}

	result = nvme_submit_admin_passthru(fd, &nvmecmd, NULL);
	if(!result)
		printf("Transcend NVME badblock count: %d\n", data[0]);

	return result;
}

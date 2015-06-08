#include <DnaTools/DnaTools.h>
#include <Private/Driver.h>

#include <Private/defines.h>

device_cmd_t dnp_rdma_commands = {
	dnp_rdma_open,
	dnp_rdma_close,
	dnp_rdma_free,
	dnp_rdma_read,
	dnp_rdma_write,
	dnp_rdma_control
};

device_cmd_t *
dnp_rdma_find_device(const char * name ATTRIBUTE_UNUSED) {

	return & dnp_rdma_commands;
}


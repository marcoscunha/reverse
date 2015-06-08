#include <DnaTools/DnaTools.h>
#include <MemoryManager/MemoryManager.h>
#include <Core/Interrupt.h>

#include <Private/Driver.h>

#ifdef DNP_RDMA_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dnp_rdma"
#include <Private/debug.h>

driver_t dnp_rdma_module = {
	"rdma",
	dnp_rdma_init_hardware,
	dnp_rdma_init_driver,
	dnp_rdma_uninit_driver,
	dnp_rdma_publish_devices,
	dnp_rdma_find_device
};

uint32_t *dnp_channels_virt_to_dev;

status_t
dnp_rdma_init_driver(void) {

	DMSG("init_driver\n");

	int32_t i = 0;
	uint32_t status;

	dnp_channels_virt_to_dev = kernel_malloc(DNP_CHANNELS_NVIRT*sizeof(uint32_t *), true);
	for(i = 0; i < DNP_CHANNELS_NVIRT; i++){
	  dnp_channels_virt_to_dev[i] = -1;
	}

	for (i = 0; i < DNP_CHANNELS_NDEV; i++) {
		dnp_channels_virt_to_dev[DNP_CHANNELS[i].id] = i;
	}


	interrupt_attach (0, DNP_SETTINGS.irq_no, 0x7 - 2, dnp_rdma_isr, false);

	dnp_reg_read(INTRCFG1_IDX, status);
	dnp_reg_write(INTRCFG1_IDX, (status & ~IRQ_ALL_IRQ_M));
	IMSG("Activating DNP interrupts: :0x%08x/0x%08x\n",
	     status, (status & ~IRQ_MASK_RDMA_M));
	
	return DNA_OK;
}






/*
 *  Copyright (c) 2010 TIMA Laboratory
 *
 *  This file is part of Rabbits.
 *
 *  Rabbits is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Rabbits is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Rabbits.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _FB_IO_DEVICE_H_
#define _FB_IO_DEVICE_H_

#include <slave_device.h>
#include <master_device.h>

enum fb_io_registers {
    FB_IO_DEVICE_WH      = 0,
    FB_IO_DEVICE_BUF     = 1,
    FB_IO_DEVICE_IRQCLR  = 2,
    FB_IO_DEVICE_CTRL    = 3,
    FB_IO_DEVICE_MODE    = 4,
    FB_IO_DATA           = 5,
    FB_IO_CANREAD        = 6,
    FB_IO_UPDATE_XY      = 7,
    FB_IO_UPDATE_WH      = 8,
    FB_IO_DEVICE_MEM     = 0x400,
};

enum fb_io_mode{
    NONE   = 0,
    /* **************************** */
    GREY   = 1,
    /* **************************** */
    RGB    = 2, /* pix[0] = R       *
                 * pix[1] = G       *
                 * pix[2] = B       */
    /* **************************** */
    BGR    = 3, /* pix[0] = B       *
                 * pix[1] = G       *
                 * pix[2] = R       */
    /* **************************** */
    ARGB   = 4, /* pix[0] = A       *
                 * pix[1] = R       *
                 * pix[2] = G       *
                 * pix[3] = B       */
    /* **************************** */
    BGRA   = 5, /* pix[0] = B       *
                 * pix[1] = G       *
                 * pix[2] = R       *
                 * pix[3] = A       */
    /* **************************** */
    YVYU   = 6, /* Packed YUV 4:2:2 */
    /* **************************** */
    YV12   = 7, /* Planar YUV 4:2:0 */
    /* **************************** */
    IYUV   = 8, /* Planar YUV 4:2:0 */
    /* **************************** */
    YV16   = 9, /* Planar YUV 4:2:2 */
};

enum fb_io_status {
    FB_IO_STOPPED = 0,
    FB_IO_RUNNING = 1,
};

typedef struct fb_io_regs fb_io_regs_t;
typedef struct fb_io_reset fb_io_reset_t;
typedef struct fb_io_resource fb_io_resource_t;

struct fb_io_resource {
    fb_io_regs_t  *regs;

    uint8_t   *mem;
    uint32_t    mem_size;
};

/*
 * internal registers status
 */
struct fb_io_regs {

    /* Registers */
    uint16_t m_image_w;
    uint16_t m_image_h;
    uint32_t m_mode;
    uint32_t m_buf_addr;
    uint32_t m_ctrl;
    uint32_t m_irqstat;

    uint16_t *m_updx, *m_updy, *m_updw, *m_updh;

    /* Status */
    uint32_t m_status;
    uint32_t m_dmaen;
    uint32_t m_irq;
    uint32_t m_disp_wrap;
};

/*
 * reset status : provided at build time.
 */
struct fb_io_reset {

    uint32_t fb_start;
    uint16_t fb_w;
    uint16_t fb_h;
    uint32_t fb_mode;
    uint32_t fb_display_on_wrap; 
};


#define FBIO_READ_BUF_SIZE           256

typedef struct
{
    int cmd;
    int data;
} fbio_read_packet;

typedef struct
{
    fbio_read_packet    read_buf[FBIO_READ_BUF_SIZE];
    unsigned long       read_pos;
    unsigned long       read_count;
    unsigned long       read_trigger;
    int                 b_read_data;
} fbio_read_state;


#define FB_IO_CTRL_START   (1 << 0)
#define FB_IO_CTRL_IRQEN   (1 << 1)
#define FB_IO_CTRL_DMAEN   (1 << 2)
#define FB_IO_CTRL_DISPLAY (1 << 3)

#define FB_IO_IRQ_DMA      (1 <<  0)
#define FB_IO_IRQ_READ     (1 << 1)
#define FB_IO_IRQ_GLOBAL   (1 << 31)
/*
 * FB_DEVICE_SLAVE
 */

class fb_io_device_slave : public slave_device
{
public:
    fb_io_device_slave (const char *_name, fb_io_resource_t *io_res,
        sc_event *irq_update, sc_event *display, 
        sc_event *start_stop, fbio_read_state *state, sc_event *ev_read);
    ~fb_io_device_slave();

public:
    /*
     *   Obtained from father
     *   void send_rsp (bool bErr);
     */
    virtual void rcv_rqst (uint32_t ofs, uint8_t be,
                           uint8_t *data, bool bWrite);

private:
    void write (uint32_t ofs, uint8_t be, uint8_t *data, bool &bErr);
    void read (uint32_t ofs, uint8_t be, uint8_t *data, bool &bErr);
    void display(void);


private:
    sc_event         *m_ev_irqupdate;
    sc_event         *m_ev_display;
    sc_event         *m_ev_start_stop;
    sc_event         *m_pev_read;
    fb_io_resource_t *m_io_res;
    fbio_read_state   *m_pread_state;

    
};

/*
 * FB_DEVICE_MASTER
 */
enum fb_io_device_master_status {
    FB_MASTER_READY        = 0,
    FB_MASTER_CMD_SUCCESS  = 1,
    FB_MASTER_CMD_ERROR    = 2,
};

class fb_io_device_master : public master_device
{
public:
    fb_io_device_master (const char *_name, uint32_t node_id);
    ~fb_io_device_master();

public:
    /*
     *   Obtained from father
     *    void send_req(unsigned char tid, unsigned long addr, unsigned char *data, 
     *      unsigned char bytes, bool bWrite);
     */
    virtual void rcv_rsp (unsigned char tid, unsigned char *data,
                          bool bErr, bool bWrite);

    int cmd_write (uint32_t addr, uint8_t *data, uint8_t nbytes);
    int cmd_read (uint32_t addr, uint8_t *data, uint8_t nbytes);


private:
    sc_event   ev_cmd_done;
    uint32_t   m_status;
    uint8_t    m_crt_tid;

    uint32_t   m_tr_rdata;
    uint8_t    m_tr_nbytes;

};

/*
 * FB_DEVICE
 */
class fb_io_device : public sc_module
{
public:
    SC_HAS_PROCESS(fb_io_device);
    fb_io_device (sc_module_name _name, uint32_t master_id, fb_io_reset_t *reset_status);
    virtual ~fb_io_device ();

public:
    slave_device  *get_slave(void);
    master_device *get_master(void);

private:
    void irq_update_thread(void);
    void display_thread(void);
    void start_stop_thread();
    void read_thread ();

    void init(int width, int height, int mode);
    void uninit(void);
    void init_viewer(void);
    void close_viewer(void);

    void convert_frame_yvyu(int idx);
    void convert_frame_yv16(int idx);
    void convert_frame_yv12(int idx);
    void convert_frame_bgr(int idx);
    void convert_frame_argb(int idx);
    void convert_frame_bgra(int idx);

public:
    sc_out<bool>        irq;

private:

    /* Interfaces */
    fb_io_device_master *master;
    fb_io_device_slave  *slave;

    /* Control and status register bank */
    fb_io_regs_t        *m_regs;
    fb_io_resource_t    *m_io_res;

    /* Simulation facilities */
    sc_event          m_ev_irq_update;
    sc_event          m_ev_display;
    sc_event          m_ev_start_stop;
    sc_event          m_ev_read;

    /* Link with viewer */
    int               m_pout;  /* pipe out to framebuffer viewer io        */
    int               m_pin;  /* pipe in from framebuffer viewer io */
    int               m_pid;   /* Xramdac PID            */
    int               m_shmid;

    /* Internal Data */
    int               m_width;
    int               m_height;
    int               m_mode;

    bool              m_dma_in_use;

    uint8_t          *m_shm_buf;
    int               m_buf_size;

    fbio_read_state   m_read_state;

#if 0
    /* YUV part ... processed */
    int               m_yuv_size;
    uint8_t          *m_yuv_image[2];  /* local buffer for YUV2RGB conversion */

    /* RGB Part ... displayed */
    int               m_rgb_components;
    int               m_rgb_size;
    uint8_t          *m_rgb_image[2];  /* shared memory with the viewer       */
#endif
};

#endif

/*
 * Vim standard variables
 * vim:set ts=4 expandtab tw=80 cindent syntax=c:
 *
 * Emacs standard variables
 * Local Variables:
 * mode: c
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */

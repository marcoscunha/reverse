/*
 * Copyright (C) 2007 TIMA Laboratory                                    
 *                                                                       
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, either version 3 of the License, or     
 * (at your option) any later version.                                   
 *                                                                       
 * This program is distributed in the hope that it will be useful,       
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
 * GNU General Public License for more details.                          
 *                                                                       
 * You should have received a copy of the GNU General Public License     
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include <Private/Driver.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>
#include <Processor/IO.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static int      first_run = 1;
static int      col __attribute__((__section__(".data"))) = 1; // cursor positions
static int      row __attribute__((__section__(".data"))) = 1; 
static int      ncols = 80, nrows = 25;
static char     attr = 0x07;

#define SCREEN_TEXT_ADDR    0xB8000
#define VGA_CRT_IC          0x3D4
#define VGA_CRT_DC  	    0x3D5
#define VGA_CRTC_CURSOR_HI	0x0E
#define VGA_CRTC_CURSOR_LO	0x0F

#define SCREEN_CHR(c) ((attr << 8) + c)

static void write_vga (unsigned char reg, unsigned int val)
{
    unsigned int v1, v2;
	v1 = reg + (val & 0xff00);
	v2 = reg + 1 + ((val << 8) & 0xff00);
	outw (v1, VGA_CRT_IC);
	outw (v2, VGA_CRT_IC);

}

static void set_cursor ()
{
    write_vga (VGA_CRTC_CURSOR_HI, (ncols * row) + col);
}

static void linefeed ()
{
    int linesize = ncols * 2;
    memcpy ((void *) SCREEN_TEXT_ADDR, (void *) (SCREEN_TEXT_ADDR + linesize), (nrows - 1) * linesize);
    memset ((void *) (SCREEN_TEXT_ADDR + (nrows - 1) * linesize), 0, linesize);
    row--;
}

static void initial_pos ()
{
    int pos;

	outb (VGA_CRTC_CURSOR_HI, VGA_CRT_IC);
	pos = inb (VGA_CRT_DC) << 8;
	outb (VGA_CRTC_CURSOR_LO, VGA_CRT_IC);
	pos += inb (VGA_CRT_DC);
	
    row = (pos / ncols) + 1;
    col = 0;

    if (row >= nrows)
        linefeed ();
}

void vga_crt_puts (char * string, int cnt)
{
    char            *p = string;
    volatile unsigned short  (*screen)[ncols] = 
        (volatile unsigned short (*)[ncols]) SCREEN_TEXT_ADDR;
    char            c;

    if (first_run)
    {
        first_run = 0;
        initial_pos ();
    }

    while (cnt-- > 0)
    {
        c = *p++;
        if (c == 0x0D)
            continue;

        if (c == 0x0A)
        {
            row++;
            col = 0;
        }
        else
        if (c == 0x0D)
        {
            screen[row][col] = SCREEN_CHR (' ');
            for (col++; (col & 0x7) && col < ncols; col ++)
                screen[row][col] = SCREEN_CHR (' ');
        }
        else
        {
            screen[row][col] = SCREEN_CHR (c);
            col++;
        }

        if (col >= ncols)
        {
            row++;
            col = 0;
        }

        if (row >= nrows)
            linefeed ();
    }
    
    set_cursor ();
}

status_t pc_tty_write (void * handler, void * source,
    int64_t offset __attribute__((unused)), int32_t * p_count)
{
    pc_tty_t * tty = (pc_tty_t *) handler;

    if ((uint32_t) &(tty->port->write) == VGA_CRT_IC)
    {
        vga_crt_puts ((char *) source, *p_count);;
    }
    else
    {
        for (int32_t i = 0; i < *p_count; i++)
        {
            cpu_write (UINT8, & (tty -> port -> write), ((char *)source)[i]);
        }
    }
    return DNA_OK;
}


void tty_print_info (char *fmt, ...)
{
    char        msg[150];
    va_list     vl;
    
    va_start(vl, fmt);
    vsprintf (msg, fmt, vl);
    vga_crt_puts (msg, strlen (msg));
    va_end(vl);
}



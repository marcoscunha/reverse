#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include "io.h"

volatile int queued_mouse_move_x = 0; // in case of mouse move lower than -128 or greater than 127 (it happened with X when mouse leave & reenter window), we have to generate several mouse moves
volatile int queued_mouse_move_y = 0; // in case of mouse move lower than -128 or greater than 127 (it happened with X when mouse leave & reenter window), we have to generate several mouse moves
int mouseX = 0, mouseY = 0;
int mouseState = 0;
int buttonChange = 0;

unsigned char third = 0;
#define MOUSE_FLAG_OVERY  0x80
#define MOUSE_FLAG_OVERX  0x40
#define MOUSE_FLAG_SIGNY  0x20
#define MOUSE_FLAG_SIGNX  0x10
#define MOUSE_FLAG_MIDDLE 0x04
#define MOUSE_FLAG_RIGHT  0x02
#define MOUSE_FLAG_LEFT   0x01
#define MOUSE_BUTTON_FLAGS (MOUSE_FLAG_LEFT | MOUSE_FLAG_MIDDLE |MOUSE_FLAG_RIGHT)

void parse_mouse_event(SDL_Event event) {
  switch (event.type) {
  case SDL_MOUSEMOTION: {
    SDL_MouseMotionEvent * motionEvent = (SDL_MouseMotionEvent *) &event;
    int dx = motionEvent->x - mouseX;
    int dy = mouseY - motionEvent->y;
    //printf("MouseMove: %d, %d\n", x, y); fflush(stdout);
    queued_mouse_move_x += dx;
    queued_mouse_move_y += dy;
    if (queued_mouse_move_x < 0)
      mouseState |= MOUSE_FLAG_SIGNX;
    else
      mouseState &= ~MOUSE_FLAG_SIGNX;

    if (queued_mouse_move_y < 0)
      mouseState |= MOUSE_FLAG_SIGNY;
    else
      mouseState &= ~MOUSE_FLAG_SIGNY;

    mouseX = motionEvent->x;
    mouseY = motionEvent->y;
    break;
  }
  case SDL_MOUSEBUTTONDOWN: {
    buttonChange=1;
    SDL_MouseButtonEvent * buttonEvent = (SDL_MouseButtonEvent *) &event;
    if (buttonEvent->button == 1)
      mouseState |= MOUSE_FLAG_LEFT;
    else if (buttonEvent->button == 2)
      mouseState |= MOUSE_FLAG_MIDDLE;
    else if (buttonEvent->button == 3)
      mouseState |= MOUSE_FLAG_RIGHT;
    else if (buttonEvent->button == 4)
      third = 0xff;
    else if (buttonEvent->button == 5)
      third = 0x01;
  }
  break;
  case SDL_MOUSEBUTTONUP: {
    buttonChange=1;
    SDL_MouseButtonEvent * buttonEvent = (SDL_MouseButtonEvent *) &event;
    if (buttonEvent->button == 1)
      mouseState &= ~MOUSE_FLAG_LEFT;
    else if (buttonEvent->button == 2)
      mouseState &= ~MOUSE_FLAG_MIDDLE;
    else if (buttonEvent->button == 3)
      mouseState &= ~MOUSE_FLAG_RIGHT;
    else if (buttonEvent->button == 4)
      third = 0x00;
    else if (buttonEvent->button == 5)
      third = 0x00;
    break;
  }
  }
}

int  create_mouse_packet() { 
  if (queued_mouse_move_x != 0 || queued_mouse_move_y != 0 || buttonChange) {
    unsigned char packets[4] = { 0, 0, 0, 0 };
    buttonChange = 0;
    // mouseState &= 0x07; // clear all but button flags
    int dx = 0, dy = 0;
    //printf("QueuedMouseMove: %d, %d\n", queued_mouse_move_x, queued_mouse_move_y); fflush(stdout);
    if (queued_mouse_move_x < -255) {
      queued_mouse_move_x += 255;
      dx = -255;
    } else if (queued_mouse_move_x > 255) {
      queued_mouse_move_x -= 255;
      dx = 255;
    } else {
      dx = queued_mouse_move_x;
      queued_mouse_move_x = 0;
    }
    if (queued_mouse_move_y < -255) {
      queued_mouse_move_y += 255;
      dy = -255;
    } else if (queued_mouse_move_y > 255) {
      queued_mouse_move_y -= 255;
      dy = 255;
    } else {
      dy = queued_mouse_move_y;
      queued_mouse_move_y = 0;
    }
    //printf("QueuedMouseMove: %d, %d\n", queued_mouse_move_x, queued_mouse_move_y); fflush(stdout);
    // if (dx < 0)
    //  mouseState |= MOUSE_FLAG_SIGNX;
    //if (dy < 0)
    //  mouseState |= MOUSE_FLAG_SIGNY;
    packets[1] = dx & 0xff;
    packets[2] = dy & 0xff;
    packets[0] = mouseState;
    packets[3] = third;
    return (packets[0] << 24) | (packets[1] << 16) | (packets[2] << 8)
        | packets[3];
  }
  return 0;
}

void handle_mouse_event (SDL_Event event) {
    parse_mouse_event (event);
    int mouse_packet, ret;
    int cmd = FBIO_CMD_MOUSE;
    
    while (0 != (mouse_packet = create_mouse_packet ()))
    {
        ret = write (STDOUT_FILENO, &cmd, 4);
        ret += write (STDOUT_FILENO, &mouse_packet, 4);
        
        //fprintf (stderr, "%s send cmd %d, data 0x%x, %d bytes written\n",
        //    __FUNCTION__, cmd, mouse_packet, ret);
    }
}

const unsigned char XKeycode2Scancode[256] = {
    // 0 - 99 -> Scancode = Keycode - 8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
    0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x20, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B,
    0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55,
    0x56,
    0x57,
    0x58,
    0x59,
    0x5A,
    0x5B,
    // 100 - 255 -> Special Keycodes
    0, 0, 0, 0, 0x1C, 0x1D, 0x35, 0, 0x38, 0, 0x47, 0x48, 0x49, 0x4B, 0x4D,
    0x4F, 0x50, 0x51, 0x52, 0x53, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x5B,
    0x5C, 0x5D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void handle_keyboard_event (SDL_Event event) {

  if (event.type != SDL_KEYUP && event.type != SDL_KEYDOWN)
    return;

  SDL_KeyboardEvent * keyEvent = (SDL_KeyboardEvent *) &event;
  int scancode = XKeycode2Scancode[keyEvent->keysym.scancode % 256];
  if (event.type == SDL_KEYUP)
    scancode |= 0x80; // key released mask

  int cmd = FBIO_CMD_KEYBOARD;
  if (keyEvent->keysym.scancode > 100) {
    int extended = 0xe0;
    // extended key : we have to generate a special scancode (0xe0) before key scancode
    write (STDOUT_FILENO, &cmd, 4);
    write (STDOUT_FILENO, &extended, 4);
  }
  write (STDOUT_FILENO, &cmd, 4);
  write (STDOUT_FILENO, &scancode, 4);

  fprintf(stderr, "fbio: keyboard scancode is 0x%08x\n", scancode);
}


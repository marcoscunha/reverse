#ifndef _FBVIEWER_IO_H_
#define _FBVIEWER_IO_H_

enum
{
    FBIO_CMD_MOUSE = 0,
    FBIO_CMD_KEYBOARD = 1,
};

void handle_mouse_event (SDL_Event event);
void handle_keyboard_event (SDL_Event event);

#endif //_FBVIEWER_IO_H_


/* SNES-style pad. Every platform maps its physical input onto these bits. */
#ifndef RR_INPUT_H
#define RR_INPUT_H

enum {
    RR_BTN_UP = 1 << 0,
    RR_BTN_DOWN = 1 << 1,
    RR_BTN_LEFT = 1 << 2,
    RR_BTN_RIGHT = 1 << 3,
    RR_BTN_A = 1 << 4,      /* attack / build / confirm */
    RR_BTN_B = 1 << 5,      /* back / online on title */
    RR_BTN_X = 1 << 6,      /* toggle build mode */
    RR_BTN_Y = 1 << 7,
    RR_BTN_L = 1 << 8,      /* previous tool */
    RR_BTN_R = 1 << 9,      /* next tool */
    RR_BTN_START = 1 << 10,
    RR_BTN_SELECT = 1 << 11,
};

#endif

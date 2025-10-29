#pragma once
#ifndef __BONZOPAD__
#define __BONZOPAD__
#include <stdio.h>
#include <windows.h>   /* required before including mmsystem.h */

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#define LAUNCHPAD_NAME "Launchpad Mini"

#define _CC 0xB0
#define _NOTE_ON  0x90
#define _NOTE_OFF 0x80

#define LED_COLOR_OFF     0x00
#define LED_COLOR_RED_1   0b0000001
#define LED_COLOR_RED_2   0b0000010
#define LED_COLOR_RED_3   0b0000011
#define LED_COLOR_GREEN_1 0b0010000
#define LED_COLOR_GREEN_2 0b0100000
#define LED_COLOR_GREEN_3 0b0110000




typedef union { unsigned long word; unsigned char data[4]; } Message;

HMIDIOUT* get_device_out();
HMIDIIN* get_device_in();

void send_message(HMIDIOUT* device, Message message);
int find_midi_device_in();
int find_midi_device_out();
void init_midi_device_out();
void init_midi_device_in(DWORD_PTR callback);
void board_off();
void board_off(HMIDIOUT* device) ;


void set_led(unsigned int y,unsigned int x,unsigned char state);
void set_led(HMIDIOUT* device,unsigned int y,unsigned int x,unsigned char state);



#endif
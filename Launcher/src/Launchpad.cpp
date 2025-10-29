#include "Launchpad.h"

/* Helper variable to map the board from y,x position */
const unsigned char board[9][9][2] = {
    {{_CC  ,0x68},{_CC  ,0x69},{_CC  ,0x6A},{_CC,  0x6B},{_CC,  0x6C},{_CC,  0x6D},{_CC,  0x6E},{_CC  ,0x6F},{0x00 ,0x00}},
    {{_NOTE_ON,0x00},{_NOTE_ON,0x01},{_NOTE_ON,0x02},{_NOTE_ON,0x03},{_NOTE_ON,0x04},{_NOTE_ON,0x05},{_NOTE_ON,0x06},{_NOTE_ON,0x07},{_NOTE_ON,0x08}},
    {{_NOTE_ON,0x10},{_NOTE_ON,0x11},{_NOTE_ON,0x12},{_NOTE_ON,0x13},{_NOTE_ON,0x14},{_NOTE_ON,0x15},{_NOTE_ON,0x16},{_NOTE_ON,0x17},{_NOTE_ON,0x18}},
    {{_NOTE_ON,0x20},{_NOTE_ON,0x21},{_NOTE_ON,0x22},{_NOTE_ON,0x23},{_NOTE_ON,0x24},{_NOTE_ON,0x25},{_NOTE_ON,0x26},{_NOTE_ON,0x27},{_NOTE_ON,0x28}},
    {{_NOTE_ON,0x30},{_NOTE_ON,0x31},{_NOTE_ON,0x32},{_NOTE_ON,0x33},{_NOTE_ON,0x34},{_NOTE_ON,0x35},{_NOTE_ON,0x36},{_NOTE_ON,0x37},{_NOTE_ON,0x38}},
    {{_NOTE_ON,0x40},{_NOTE_ON,0x41},{_NOTE_ON,0x42},{_NOTE_ON,0x43},{_NOTE_ON,0x44},{_NOTE_ON,0x45},{_NOTE_ON,0x46},{_NOTE_ON,0x47},{_NOTE_ON,0x48}},
    {{_NOTE_ON,0x50},{_NOTE_ON,0x51},{_NOTE_ON,0x52},{_NOTE_ON,0x53},{_NOTE_ON,0x54},{_NOTE_ON,0x55},{_NOTE_ON,0x56},{_NOTE_ON,0x57},{_NOTE_ON,0x58}},
    {{_NOTE_ON,0x60},{_NOTE_ON,0x61},{_NOTE_ON,0x62},{_NOTE_ON,0x63},{_NOTE_ON,0x64},{_NOTE_ON,0x65},{_NOTE_ON,0x66},{_NOTE_ON,0x67},{_NOTE_ON,0x68}},
    {{_NOTE_ON,0x70},{_NOTE_ON,0x71},{_NOTE_ON,0x72},{_NOTE_ON,0x73},{_NOTE_ON,0x74},{_NOTE_ON,0x75},{_NOTE_ON,0x76},{_NOTE_ON,0x77},{_NOTE_ON,0x78}}
};

/* Windows MIDI device in/out */
HMIDIIN   device_in=NULL;
HMIDIIN* get_device_in() {return &device_in;}

HMIDIOUT  device_out=NULL;
HMIDIOUT* get_device_out() { return &device_out;}


void send_message(HMIDIOUT* device, Message message) {
    /*
        Send Midi Message
    */
    int flag = midiOutShortMsg(*device, message.word);
    if (flag != MMSYSERR_NOERROR) {
        printf("Warning: MIDI Output is not open.\n");
    }
}

int find_midi_device_in(){
    UINT numinDev = midiInGetNumDevs();
    if (!numinDev){
        return -1;
    } 
    for(int dev=0;dev<numinDev;dev++){
        MIDIINCAPS capacity;
        MMRESULT result = midiInGetDevCaps(dev, &capacity,sizeof(capacity));
        if( strstr(capacity.szPname, LAUNCHPAD_NAME) != NULL ) return dev;
        
    }
    return -1;
}

int find_midi_device_out() {
    UINT numOutDev = midiOutGetNumDevs();
    if (!numOutDev){
        return -1;
    } 
    for(int dev=0;dev<numOutDev;dev++){
        MIDIOUTCAPS capacity;
        MMRESULT result = midiOutGetDevCaps(dev, &capacity,sizeof(capacity));
        if( strstr(capacity.szPname, LAUNCHPAD_NAME) != NULL ) return dev;
    }
    return -1;
}

void init_midi_device_out(){
    int  launchpad_interface = find_midi_device_out();
    MMRESULT result = midiOutOpen(&device_out,launchpad_interface,0,0,CALLBACK_NULL);
    if (result != MMSYSERR_NOERROR) {
        printf("[Launchpad] Error opening MIDI Output.\n");
    }
}
void init_midi_device_in(DWORD_PTR callback) {
    int  launchpad_interface = find_midi_device_in();
    HMIDIIN* device_in = get_device_in();
    MMRESULT result =midiInOpen(device_in, launchpad_interface,(DWORD_PTR) callback, 0, CALLBACK_FUNCTION);
    if (result != MMSYSERR_NOERROR) {
        printf("[Launchpad] Error opening MIDI Input.\n");

    } 
    result = midiInStart(*device_in);
    if (result != MMSYSERR_NOERROR) {
        printf("[Launchpad] Error midiInStart MIDI Input.\n");

    }
}
/*====================*/
void board_off() {
    if(device_out ==NULL) {
        init_midi_device_out();
    }
    Message message;
    message.data[0] = 0xB0;
    message.data[1] = 0x00 ;  
    message.data[2] = 0x00;   
    message.data[3] = 0;  
    send_message(&device_out,message);
}
void board_off(HMIDIOUT* device) {
    Message message;
    message.data[0] = 0xB0;
    message.data[1] = 0x00 ;  
    message.data[2] = 0x00;   
    message.data[3] = 0;  
    send_message(device,message);
}


void set_led(unsigned int y,unsigned int x,unsigned char state) {
    if(device_out ==NULL) {
        init_midi_device_out();
    }
    Message message;
    message.data[0] = board[y][x][0];
    message.data[1] = board[y][x][1];   
    message.data[2] = state;   
    message.data[3] = 0;  
    send_message(&device_out,message);
}
void set_led(HMIDIOUT* device,unsigned int y,unsigned int x,unsigned char state) {
    Message message;
    message.data[0] = board[y][x][0];
    message.data[1] = board[y][x][1];   
    message.data[2] = state;   
    message.data[3] = 0;  
    send_message(device,message);
}



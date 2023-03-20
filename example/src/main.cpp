#include <Arduino.h>


#define RX_BUF_SIZE 1024
uint8_t uartRxBuff[RX_BUF_SIZE];
int rxPos = 0;
int cmdLength = 0;
uint8_t cmdType = 0;
long lastRxReceive = 0;

String deviceType[] = { "UNKNOWN", "POINTER",  "MOUSE",  "RESERVED",   "JOYSTICK",
                        "GAMEPAD", "KEYBOARD", "KEYPAD", "MULTI_AXIS", "SYSTEM" };
String keyboardstring;


typedef enum {
    MSG_TYPE_CONNECTED = 0x01,
    MSG_TYPE_DISCONNECTED = 0x02,
    MSG_TYPE_ERROR = 0x03,
    MSG_TYPE_DEVICE_POLL = 0x04,
    MSG_TYPE_DEVICE_STRING = 0x05,
    MSG_TYPE_DEVICE_INFO = 0x06,
    MSG_TYPE_HID_INFO = 0x07,
    MSG_TYPE_STARTUP = 0x08,
} msg_type_t;


typedef struct {
    uint8_t prefix;
    uint16_t cmdLength;
    msg_type_t msgType : 8;
    uint8_t devType;
    uint8_t device;
    uint8_t endpoint;
    uint16_t idVendor;
    uint16_t idProduct;
    uint8_t command[];
} __attribute__((packed)) packetthing_t;

void printHex(uint8_t* buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        Serial.print("0x");
        Serial.print(buf[i], HEX);
        Serial.print(" ");
    }
}

void filterCommand(int buffLength, packetthing_t* packet)
{
    printHex((uint8_t*)packet, buffLength);
    Serial.println();
    switch (packet->msgType) {
        case MSG_TYPE_CONNECTED:
            Serial.print("Device Connected on port");
            Serial.println(packet->device);
            break;
        case MSG_TYPE_DISCONNECTED:
            Serial.print("Device Disconnected on port");
            Serial.println(packet->device);
            break;
        case MSG_TYPE_ERROR:
            Serial.print("Device Error ");
            Serial.print(packet->device);
            Serial.print(" on port ");
            Serial.println(packet->devType);
            break;
        case MSG_TYPE_DEVICE_POLL:
            Serial.print("Device HID Data from port: ");
            Serial.print(packet->device);
            Serial.print(" , Length: ");
            Serial.print((int)packet->cmdLength);
            Serial.print(" , Type: ");
            Serial.print(deviceType[packet->devType]);
            Serial.printf(" , ID: %04X:%04X", packet->idVendor, packet->idProduct);
            Serial.print(" ,    ");
            printHex(packet->command, packet->cmdLength);
            Serial.println();
            break;
        case MSG_TYPE_DEVICE_STRING:
            Serial.print("Device String port ");
            Serial.print(packet->devType);
            Serial.print(" Name: ");
            for (size_t i = 0; i < packet->cmdLength; i++) {
                Serial.write(packet->command[i]);
            }
            Serial.println();
            break;
        case MSG_TYPE_DEVICE_INFO:
            Serial.print("Device info from port");
            Serial.print(packet->device);
            Serial.print(", Descriptor: ");
            printHex(packet->command, packet->cmdLength);
            Serial.println();
            break;
        case MSG_TYPE_HID_INFO:
            Serial.print("HID info from port");
            Serial.print(packet->device);
            Serial.print(", Descriptor: ");
            printHex(packet->command, packet->cmdLength);
            Serial.println();
            break;
        case MSG_TYPE_STARTUP:
            Serial.println("USB host ready");
            break;
    }
}

void setup(void)
{
    Serial.begin(115200);
    Serial2.begin(1000000, SERIAL_8N1);
    Serial.println("OK There");
}

void loop()
{
    while (Serial2.available()) {
        lastRxReceive = millis();
        //Serial.print("h0x");//Only for Debug
        //Serial.print(Serial2.peek(),HEX);//Only for Debug
        //Serial.print(" ");//Only for Debug
        uartRxBuff[rxPos] = Serial2.read();
        if (rxPos == 0 && uartRxBuff[rxPos] == 0xFE) {
            cmdType = 1;
        } else if (rxPos == 1 && cmdType == 1) {
            cmdLength = uartRxBuff[rxPos];
        } else if (rxPos == 2 && cmdType == 1) {
            cmdLength += (uartRxBuff[rxPos] << 8);
            //printf( "Length: %i\n", cmdLength);//Only for Debug
        } else if (cmdType == 0 && uartRxBuff[rxPos] == '\n') {
            printf("No COMMAND Received\n");
            for (uint8_t i = 0; i < rxPos; i++) {
                printf("0x%02X ", uartRxBuff[i]);
            }
            printf("\n");
            rxPos = 0;
            cmdType = 0;
        }
        if (rxPos > 0 && rxPos == cmdLength + 11 && cmdType || rxPos > RX_BUF_SIZE) {
            filterCommand(rxPos, (packetthing_t*)uartRxBuff);
            rxPos = 0;
            cmdType = 0;
        } else {
            rxPos++;
        }
    }
    rxPos = 0;

    if (Serial.available()) {
        Serial2.write(Serial.read());
    }
}

#include <Arduino.h>
#include <ESP32CAN.h>
#include <CAN_config.h>
#include "cantp.h"

CAN_device_t CAN_cfg; 
const int rx_queue_size = 10;  
Cantp_HandlerStruct Handler;

bool_t CanTx(uint32_t id, uint8_t* msg, uint32_t size)
{
    CAN_frame_t tx_frame;
    memcpy(tx_frame.data.u8,msg,size);
    tx_frame.MsgID = id;
    tx_frame.FIR.B.DLC = size;
    tx_frame.FIR.B.FF = CAN_frame_std;
    ESP32Can.CANWriteFrame(&tx_frame);
    return 0;
}

bool_t CanRx(uint32_t* id, uint8_t* msg, uint32_t* size)
{
    CAN_frame_t rx_frame;
    if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 0) == pdTRUE)
    {
        *id = rx_frame.MsgID;
        *size = rx_frame.FIR.B.DLC;
        memcpy(msg,rx_frame.data.u8,8);
        return 1;
    }
    return 0;
}


void setup()
{
    Serial.begin(115200);
    Serial.println("Basic Demo - ESP32-Arduino-CAN");
    CAN_cfg.speed = CAN_SPEED_500KBPS;
    CAN_cfg.tx_pin_id = GPIO_NUM_5;
    CAN_cfg.rx_pin_id = GPIO_NUM_4;
    CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
    ESP32Can.CANInit();
    Cantp_Register(&Handler,CanTx,CanRx,0x456,0x123);
}

void loop()
{
    uint32_t id;
    uint32_t size;
    uint8_t *data;
    Cantp_RxTask(&Handler, CALL_TASK, 0, NULL, 0);
    Cantp_TxTask(&Handler);
    //方案一：以轮询方式收取数据
    // if(Cantp_Rx(&Handler,&id, &data, &size) == TRUE)
    // {
    //     Cantp_Tx(&Handler,0x000, data, size);
    // }
    delay(10);
}


//方案二：以回调方式收取数据
void Cantp_RxCallback(Cantp_HandlerStruct* Handler, uint32_t id, uint8_t* msg, uint32_t size)
{
    Cantp_Tx(Handler,0x7FF, msg, size);
}
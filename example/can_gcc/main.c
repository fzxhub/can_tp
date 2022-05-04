#include "stdio.h"
#include "cantp.h"

bool_t CanTx(uint32_t id, uint8_t* msg, uint32_t len)
{
    printf("发送数据：");
    printf("ID:%x - ", id);
    for(int i = 0; i < len; i++)
        printf("%02X ",*(msg+i));
    printf("\r\n");
    return TRUE;
}

Cantp_HandlerStruct Cantp_Handler;

int main(void)
{
    uint8_t data[256] = "01234567890123456789012345678901234567890123456789\
                         01234567890123456789012345678901234567890123456789";
    Cantp_Register(&Cantp_Handler, CanTx, NULL, 0x123, 0x456);
    //发送ID在管理列表，在首帧或者帧块时必须收到流控帧才能继续发送，否则中断发送
    Cantp_TxBlocking(&Cantp_Handler,0x456, data, 5);
    Cantp_TxBlocking(&Cantp_Handler,0x456, data, 18);
    //发送ID在不管理列表，直接全部发送
    Cantp_TxBlocking(&Cantp_Handler,0x189, data, 22);
}

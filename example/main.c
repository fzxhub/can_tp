#include "stdio.h"
#include "cantp.h"

bool Send(uint32_t id, uint8_t* msg, uint32_t len)
{
    printf("发送数据：");
    printf("ID:%x - ", id);
    for(int i = 0; i < len; i++)
        printf("%02X ",*(msg+i));
    printf("\r\n");

    return TRUE;
}

void Isotp_SendCall(bool status)
{
    printf("发送OK\r\n");
}


int  main(void)
{
    uint8_t data[32] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
                        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
                        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
                        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08
                        };


    Cantp_register(Send, NULL);
    Cantp_BlockingTx(0x100,data,20);
}

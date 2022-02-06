#include "stdio.h"
#include "isotp.h"

bool Send(uint32_t id, uint8_t* msg, uint32_t len)
{
    printf("发送数据：");
    printf("ID:%x - ", id);
    for(int i = 0; i < len; i++)
        printf("%02X ",*(msg+i));
    printf("\r\n");

    return TRUE;
}

int  main(void)
{
    uint8_t data[32] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
                        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
                        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
                        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08
};

    //Isotp_ConsecutiveSend(0x100,data,8,Send);

    //Isotp_FlowControlSend(0x111,ISOTP_FLOW_STATUS_CONTINUE,8,0,Send);

    Isotp_SendAsyn(0x100,data,32,Send);


    while(1)
    {
        Isotp_SendTask(1);
    }



}
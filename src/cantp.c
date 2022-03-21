#include "cantp.h"
#include "string.h"
#include "stdio.h"

/**********************************************************************************************/
//发送部分
/**********************************************************************************************/

//功能：单帧发送
//id：发送的CAN帧ID
//msg:要发送的消息
//size：消息大小
//CanSend：CAN发送函数
//return：TRUE：成功；FALSE：失败
bool Cantp_SingleSend(uint32_t id, uint8_t* msg, uint32_t size, Cantp_CanSend CanSend)
{
    //帧数据暂存
    uint8_t temp_data[CANTP_FRAME_BYTE];
    uint8_t temp_size = 0;
    //长度值大于7，表示单帧传输不了
    if(size > CANTP_FRAME_BYTE-1) return FALSE;
    //发送函数为空，表示不能传输
    if(CanSend == NULL)  return FALSE;
    //第0字节表示单帧和长度
    temp_data[0] = (CANTP_SINGLE_FRAME<<4) | size;
    //拷贝数据到第1字节及之后
    memcpy(&temp_data[1], msg, size);
    //填充无效内容
    if(CANTP_FRAME_FILL == TRUE)
    {
        memset(&temp_data[size+1], CANTP_FRAME_VOID, CANTP_FRAME_BYTE - size - 1);
        temp_size = CANTP_FRAME_BYTE;
    }
    else temp_size = size + 1;
    
    //发送CAN帧数据
    return CanSend(id,temp_data,temp_size);
}
//功能：首帧发送
//id：发送的CAN帧ID
//msg:要发送的消息
//size：消息大小
//CanSend：CAN发送函数
//return：TRUE：成功；FALSE：失败
bool Cantp_FirstSend(uint32_t id, uint8_t* msg, uint32_t sizes, Cantp_CanSend CanSend)
{
    //大小大于消息大小不能传输
    if(sizes > CANTP_MESSAGE_BYTE) return FALSE;
    //发送函数为空，表示不能传输
    if(CanSend == NULL)  return FALSE;
    //帧数据暂存
    uint8_t temp[CANTP_FRAME_BYTE];
    //第0、1字节表示首帧和长度
    temp[0] = (CANTP_FIRST_FRAME<<4) | (sizes>>8);
    temp[1] = sizes;
    //拷贝数据到第2字节及之后全部填充
    memcpy(&temp[2], msg, CANTP_FRAME_BYTE-2);
    //发送CAN帧数据
    return CanSend(id,temp,CANTP_FRAME_BYTE);
}

//功能：连续帧发送
//id：发送的CAN帧ID
//msg:要发送的消息
//size：还剩余消息大小
//CanSend：CAN发送函数
//return：TRUE：成功；FALSE：失败
bool Cantp_ConsecutiveSend(uint32_t id, uint8_t* msg, uint32_t size, Cantp_CanSend CanSend)
{
    //连续帧计数
    static uint8_t  time = 0x01;
    //帧数据暂存
    uint8_t temp_data[CANTP_FRAME_BYTE];
    uint8_t temp_size = 0;
    //大小大于消息大小不能传输
    if(size > CANTP_MESSAGE_BYTE) return FALSE;
    //发送函数为空，表示不能传输
    if(CanSend == NULL)  return FALSE;
    
    //第0字节
    temp_data[0] = (CANTP_CONSECUTIVE_FRAME<<4) | (time&0x0f);
    //如果剩余的大小大于单帧能传输的数据
    if(size > CANTP_FRAME_BYTE-1)
    {
        memcpy(&temp_data[1], msg, CANTP_FRAME_BYTE-1);
        time++;
        temp_size = CANTP_FRAME_BYTE;
    }    
    else
    {
        memcpy(&temp_data[1], msg, size);
        //填充无效内容
        if(CANTP_FRAME_FILL == TRUE)
        {
            memset(&temp_data[size+1], CANTP_FRAME_VOID, CANTP_FRAME_BYTE - size - 1);
            temp_size = CANTP_FRAME_BYTE;
        }
        else temp_size = size + 1;
        time = 0x01;
    }
    //发送CAN帧数据
    return CanSend(id,temp_data,temp_size);
}

//功能：流控帧发送
//id：发送的CAN帧ID
//status:本端状态
//block：回复流控帧块大小
//Stmin：本端消息间隔最小时间
//CanSend：CAN发送函数
//return：TRUE：成功；FALSE：失败
bool Cantp_FlowControlSend(uint32_t id, Cantp_FlowStatus status, uint8_t block , uint8_t Stmin, Cantp_CanSend CanSend)
{
    //帧数据暂存
    uint8_t temp_data[CANTP_FRAME_BYTE];
    uint8_t temp_size = 0;
    //第0字节
    temp_data[0] = (CANTP_CONSECUTIVE_FRAME<<4) & (status);
    //第1字节
    temp_data[1] = block;
    //第2字节  0x00-0x7F:0-127毫秒  0xF1-0xF9:0.1-0.9毫秒
    if((Stmin <= 0x7F) || (Stmin >= 0xF1 && Stmin <= 0xF9))
        temp_data[2] = Stmin;
    //填充无效内容
    if(CANTP_FRAME_FILL == TRUE)
    {
        memset(&temp_data[3], CANTP_FRAME_VOID, CANTP_FRAME_BYTE-3);
        temp_size = CANTP_FRAME_BYTE;
    }
    else  temp_size = 3;
    //发送CAN帧   
    return CanSend(id,temp_data,temp_size);
}

//阻塞式发送
bool Cantp_Send(uint32_t id, uint8_t* msg, uint32_t size, Cantp_CanSend CanSend)
{
    static uint32_t temp_size = 0;
    static uint8_t temp_flow = 0;
    static uint8_t temp_stmin = 0;
    if(size <= 7)
        return Cantp_SingleSend(id,msg,size,CanSend); 
    else
    {
        Cantp_FirstSend(id,msg,6,CanSend);
        temp_size = temp_size + 6;
        //发送延时函数，用于发送间隔(后续加上远端的能力)
        DELAY_US();
        while(1)
        {
            //收到流控帧判断(后面加)
            if(1)
            {
            }
            Cantp_ConsecutiveSend(id,msg+temp_size,size-temp_size,CanSend);
            if(size > temp_size + 7)  temp_size = temp_size + 7;
            else
            {
                temp_size = 0;
                break;
            } 
            //发送延时函数，用于发送间隔(后续加上远端的能力)
            DELAY_US();
        }
    }
    return TRUE;     
}

//异步发送
bool Cantp_SendAsyn(uint32_t id, uint8_t* msg, uint32_t size, Cantp_CanSend CanSend)
{
    if(size > CANTP_MESSAGE_BYTE) return FALSE;
    Send_Struct.send_id = id;
    Send_Struct.data = msg;
    Send_Struct.size = size;
    Send_Struct.now_size = 0;
    Send_Struct.runing = TRUE;
    Send_Struct.send = CanSend;
    return TRUE;
}
//回调式轮训函数
void Cantp_SendTask(float time)
{
    if(Send_Struct.runing == FALSE) return;
    
    if(Send_Struct.now_size == 0)
    {
        if(Send_Struct.size <= 7)
            Cantp_SingleSend(Send_Struct.send_id,Send_Struct.data,Send_Struct.size,Send_Struct.send); 
        else
        {
            Cantp_FirstSend(Send_Struct.send_id,Send_Struct.data,6,Send_Struct.send);
            Send_Struct.now_size = Send_Struct.now_size + 6;
        }
    }
    else
    {
        Cantp_ConsecutiveSend(Send_Struct.send_id,Send_Struct.data+Send_Struct.now_size,
        Send_Struct.size-Send_Struct.now_size,Send_Struct.send);
        if(Send_Struct.size > Send_Struct.now_size + 7)  
            Send_Struct.now_size = Send_Struct.now_size + 7;
        else
        {
            Send_Struct.now_size = 0;
            Send_Struct.runing = FALSE;
            Cantp_SendCall(TRUE);
        } 
    }
}

WEAK void Cantp_SendCall(bool status){}


/**********************************************************************************************/
//接收部分
/**********************************************************************************************/

void Cantp_Rece(uint32_t id, Cantp_Message* tpmsg, Cantp_CanRece CanRece, Cantp_CanSend CanSend)
{

    uint8_t* msg;
    uint32_t size;
    static uint32_t msgs_size = 0;
    static uint32_t flow_size = 0;
    static uint32_t msgs_all_size = 0;
    if (CanRece(&id, msg, &size) == TRUE)
    {
        switch(((*msg)&0xf0)>>4)
        {
            case CANTP_SINGLE_FRAME:
            {
                tpmsg->size = (*msg)&0x0F;
                memcpy(tpmsg->payload,msg,tpmsg->size);
                tpmsg->completed = TRUE;
                tpmsg->multi = FALSE;    
            }break;
            case CANTP_FIRST_FRAME:
            {
                msgs_all_size = (((*msg)&0x0F)<<8) + *(msg+1);
                //首帧数据只剩下6个字节有效数据
                memcpy(tpmsg->payload,msg+2,6);
                tpmsg->completed = FALSE;
                tpmsg->multi = TRUE;
                msgs_size = 6;
                flow_size = 6;
                Cantp_FlowControlSend(id,CANTP_FLOW_STATUS_CONTINUE,CANTP_FLOW_BLOCK,CANTP_FLOW_TIMES,CanSend);
            }break;
            case CANTP_CONSECUTIVE_FRAME:
            {
                // if(msgs_all_size > )
                // {
                    
                // }
                
            }break;
            case CANTP_FLOWCONTROL_FRAME:
            {
                //Cantp_Remote.block = *(msg+1);
                //Cantp_Remote.status = *(msg) & 0x0F;
                //Cantp_Remote.stmin = *(msg+2);
            }break;
            default: break;
        }
    }
}


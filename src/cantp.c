#include "cantp.h"
#include "string.h"
#include "stdio.h"

/***************************************************************************************************************/
//内部发送函数，发送分割函数
/***************************************************************************************************************/
/*
 * 功能：单帧发送
 * id：发送的CAN帧ID
 * msg:要发送的消息
 * size：消息大小
 * CanSend：CAN发送函数
 * return：TRUE：成功；FALSE：失败
*/
bool Cantp_Single(uint32_t id, uint8_t* msg, uint32_t size, Cantp_CanTx send)
{
    //帧数据暂存
    uint8_t temp_data[CANTP_FRAME_BYTE];
    uint8_t temp_size = 0;
    //长度值大于7，表示单帧传输不了
    if(size > CANTP_FRAME_BYTE-1) return FALSE;
    //发送函数为空，表示不能传输
    if(send == NULL)  return FALSE;
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
    return send(id,temp_data,temp_size);
}
/*
 * 功能：首帧发送
 * id：发送的CAN帧ID
 * msg:要发送的消息
 * size：消息大小
 * send：CAN发送函数
 * return：TRUE：成功；FALSE：失败
*/
bool Cantp_First(uint32_t id, uint8_t* msg, uint32_t sizes, Cantp_CanTx send)
{
    //大小大于消息大小不能传输
    if(sizes > CANTP_MESSAGE_BYTE) return FALSE;
    //发送函数为空，表示不能传输
    if(send == NULL)  return FALSE;
    //帧数据暂存
    uint8_t temp[CANTP_FRAME_BYTE];
    //第0、1字节表示首帧和长度
    temp[0] = (CANTP_FIRST_FRAME<<4) | (sizes>>8);
    temp[1] = sizes;
    //拷贝数据到第2字节及之后全部填充
    memcpy(&temp[2], msg, CANTP_FRAME_BYTE-2);
    //发送CAN帧数据
    return send(id,temp,CANTP_FRAME_BYTE);
}
/*
 * 功能：连续帧发送
 * id：发送的CAN帧ID
 * msg:要发送的消息
 * size：消息大小
 * send：CAN发送函数
 * return：TRUE：成功；FALSE：失败
*/
bool Cantp_Consecutive(uint32_t id, uint8_t* msg, uint32_t size, Cantp_CanTx send)
{
    //连续帧计数
    static uint8_t  time = 0x01;
    //帧数据暂存
    uint8_t temp_data[CANTP_FRAME_BYTE];
    uint8_t temp_size = 0;
    //大小大于消息大小不能传输
    if(size > CANTP_MESSAGE_BYTE) return FALSE;
    //发送函数为空，表示不能传输
    if(send == NULL)  return FALSE;
    
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
    return send(id,temp_data,temp_size);
}
/*
 * 功能：流控帧发送
 * id：发送的CAN帧ID
 * status:本端状态
 * block：回复流控帧块大小
 * Stmin：本端消息间隔最小时间
 * CanSend：CAN发送函数
 * return：TRUE：成功；FALSE：失败
*/
bool Cantp_FlowControl(uint32_t id, Cantp_FlowStatus status, uint8_t block , uint8_t stmin, Cantp_CanTx send)
{
    //帧数据暂存
    uint8_t temp_data[CANTP_FRAME_BYTE];
    uint8_t temp_size = 0;
    //发送函数为空，表示不能传输
    if(send == NULL)  return FALSE;
    //第0字节
    temp_data[0] = (CANTP_CONSECUTIVE_FRAME<<4) & (status);
    //第1字节
    temp_data[1] = block;
    //第2字节  0x00-0x7F:0-127毫秒  0xF1-0xF9:0.1-0.9毫秒
    if((stmin <= 0x7F) || (stmin >= 0xF1 && stmin <= 0xF9))
        temp_data[2] = stmin;
    //填充无效内容
    if(CANTP_FRAME_FILL == TRUE)
    {
        memset(&temp_data[3], CANTP_FRAME_VOID, CANTP_FRAME_BYTE-3);
        temp_size = CANTP_FRAME_BYTE;
    }
    else  temp_size = 3;
    //发送CAN帧   
    return send(id,temp_data,temp_size);
}

/***************************************************************************************************************/
//对外接口函数
/***************************************************************************************************************/
/*
 * 功能：注册CAN的发送接收函数
 * tx：发送函数指针
 * rx：接收函数指针
*/
void Cantp_register(Cantp_HandlerStruct* Handler, Cantp_CanTx tx, Cantp_CanRx rx, uint32_t rxid, uint32_t txid)
{
    Handler->CanApi.rx = rx;
    Handler->CanApi.tx = tx;
    Handler->Local.stmin = 0;
    Handler->Local.block = 8;
    Handler->RxIdList[0] = rxid;
    Handler->TxIdList[0] = txid;
}
/*
 * 功能：阻塞式发送
 * id：发送的CAN帧ID
 * msg：发送数据
 * size：发送大小
*/
bool Cantp_TxBlocking(Cantp_HandlerStruct* Handler, uint32_t id, uint8_t* msg, uint32_t size)
{
    uint32_t temp_size = 0;
    uint8_t temp_flow = 0;
    uint8_t temp_stmin = 0;
    int32_t temp_index = -1;
    int32_t temp_times = 0;
    //索引ID是否在检测表中
    for(int i = 0; i < CANTP_ID_MAX; i++)
    {
        if(id == Handler->TxIdList[i])
        {
            temp_index = i;
            break;
        }
    }
    if(size <= 7)
        return Cantp_Single(id,msg,size,Handler->CanApi.tx); 
    else
    {
        Cantp_First(id,msg,size,Handler->CanApi.tx);
        temp_size = temp_size + 6;
        temp_flow++;
        while(1)
        {
            if((temp_index != -1) && (Handler->Rxmsg[temp_index].flow == TRUE))
            {
                Handler->Rxmsg[temp_index].flow = FALSE;
                goto SEND2;
            }
            else 
            {
                DELAY_US();
                temp_times++;
                if(temp_times >= 1000) return FALSE;
            }
        }        
    SEND2:
        while(1)
        {
            if(temp_flow%CANTP_FLOW_BLOCK==0)
            {
                while(1)
                {
                    if((temp_index != -1) && (Handler->Rxmsg[temp_index].flow == TRUE))
                    {
                        Handler->Rxmsg[temp_index].flow = FALSE;
                        goto CONTINUE;
                    }
                    else 
                    {
                        DELAY_US();
                        temp_times++;
                        if(temp_times >= 1000) return FALSE;
                    }
                }
            }
            CONTINUE:
            Cantp_Consecutive(id,msg+temp_size,size-temp_size,Handler->CanApi.tx);
            temp_flow++;
            if(size > temp_size + 7)  temp_size = temp_size + 7;
            else
            {
                temp_size = 0;
                break;
            }
            DELAY_US();
        }
    }
    return TRUE;     
}
/*
 * 功能：非阻塞式发送
 * id：发送的CAN帧ID
 * msg：发送数据
 * size：发送大小
*/
bool Cantp_Tx(Cantp_HandlerStruct* Handler, uint32_t id, uint8_t* msg, uint32_t size)
{
    if(size > CANTP_MESSAGE_BYTE) return FALSE;

    Handler->TxState.id = id;
    Handler->TxState.data = msg;
    Handler->TxState.allsize = size;
    Handler->TxState.size = 0;
    Handler->TxState.runing = TRUE;

    return TRUE;
}
/*
 * 功能：发送任务函数
 * time：调用周期时间
*/
void Cantp_TxTask(Cantp_HandlerStruct* Handler)
{
    int32_t temp_index = -1;
    if(Handler->TxState.runing == FALSE) return;
    //索引ID是否在检测表中
    for(int i = 0; i < CANTP_ID_MAX; i++)
    {
        if(Handler->TxState.id == Handler->TxIdList[i])
        {
            temp_index = i;
            break;
        }
    } 
    //发送
    if(Handler->TxState.size == 0) //已经发送大小为0，表示才开始发送
    {
        if(Handler->TxState.allsize <= 7) //小于7字节要发送，单帧即可发送完
            Cantp_Single(Handler->TxState.id,Handler->TxState.data,Handler->TxState.allsize,Handler->CanApi.tx); 
        else
        {
            Cantp_First(Handler->TxState.id,Handler->TxState.data,Handler->TxState.allsize,Handler->CanApi.tx);
            Handler->TxState.size = Handler->TxState.size + 6;
            Handler->TxState.frame++;
        }
    }
    else
    {
        if(Handler->TxState.frame%CANTP_FLOW_BLOCK == 0)
        {
            if((temp_index != -1) && (Handler->Rxmsg[temp_index].flow == FALSE)) 
            {
                Handler->Rxmsg[temp_index].flow = FALSE;
                return;
            }
        }
        Cantp_Consecutive(Handler->TxState.id,Handler->TxState.data+Handler->TxState.size,
        Handler->TxState.allsize-Handler->TxState.size,Handler->CanApi.tx);
        if(Handler->TxState.allsize > Handler->TxState.size + 7)  
            Handler->TxState.size = Handler->TxState.size + 7;
        else
        {
            Handler->TxState.size = 0;
            Handler->TxState.runing = FALSE;
            Cantp_TxCallback(Handler, Handler->TxState.id);
        } 
    }
}


/**********************************************************************************************/
//接收部分
/**********************************************************************************************/

void Cantp_RxTask(Cantp_HandlerStruct* Handler, Cantp_CallWay way,uint32_t id, uint8_t* msg, uint32_t size)
{
    //暂存消息
    uint8_t msg_temp[CANTP_FRAME_BYTE];
    //以下指向实际数据的中间数据
    uint8_t* msg_c;
    uint32_t size_c;
    uint32_t id_c;

    if(way == CALL_BACK)
    {
        msg_c = msg;
        size_c = size;
        id_c = id;
    }
    else if(way == CALL_TASK)
    {
        msg_c = msg_temp;
        if(Handler->CanApi.rx(&id_c, msg, &size_c) == FALSE)
            return;
    }
    for(int i = 0; i < CANTP_ID_MAX; i++)
    {
        if(Handler->RxIdList[i] == id_c)
        {
            switch(((*msg)&0xf0)>>4)
            {
                case CANTP_SINGLE_FRAME:
                {
                    Handler->Rxmsg[i].size = (*msg)&0x0F;
                    memcpy(Handler->Rxmsg[i].payload,msg,Handler->Rxmsg[i].size);
                    Handler->Rxmsg[i].completed = TRUE;
                    Handler->Rxmsg[i].multi = FALSE;
                    Cantp_RxCallback(Handler,Handler->RxIdList[i],Handler->Rxmsg[i].payload,Handler->Rxmsg[i].size);                        
                }break;
                case CANTP_FIRST_FRAME:
                {
                    Handler->Rxmsg[i].allsize = (((*msg)&0x0F)<<8) + *(msg+1);
                    //首帧数据只剩下6个字节有效数据
                    memcpy(Handler->Rxmsg[i].payload,msg+2,6);
                    Handler->Rxmsg[i].completed = FALSE;
                    Handler->Rxmsg[i].multi = TRUE;
                    Handler->Rxmsg[i].size = 6;
                    Handler->Rxmsg[i].frame = 1;
                    Cantp_FlowControl(Handler->TxIdList[i],CANTP_FLOW_STATUS_CONTINUE,CANTP_FLOW_BLOCK,CANTP_FLOW_TIMES,Handler->CanApi.tx);
                }break;
                case CANTP_CONSECUTIVE_FRAME:
                {
                    if(Handler->Rxmsg[i].allsize >= Handler->Rxmsg[i].size + 7)
                    {
                        memcpy(Handler->Rxmsg[i].payload + Handler->Rxmsg[i].size, msg+1, 7);
                        Handler->Rxmsg[i].size += 7;
                        Handler->Rxmsg[i].frame++;
                    }
                    else
                    {
                        memcpy(Handler->Rxmsg[i].payload + Handler->Rxmsg[i].size, msg+1, Handler->Rxmsg[i].allsize - Handler->Rxmsg[i].size);
                        Handler->Rxmsg[i].size = Handler->Rxmsg[i].allsize;
                        Handler->Rxmsg[i].frame++;
                    }
                    if(Handler->Rxmsg[i].allsize == Handler->Rxmsg[i].size)
                    {
                        Handler->Rxmsg[i].completed = TRUE;
                        Cantp_RxCallback(Handler,Handler->RxIdList[i],Handler->Rxmsg[i].payload,Handler->Rxmsg[i].size);
                    }
                    //表示每收到BLOCK整数倍时需要发送流控帧
                    if(Handler->Rxmsg[i].frame%CANTP_FLOW_BLOCK == 0)
                    {
                        Cantp_FlowControl(Handler->TxIdList[i],CANTP_FLOW_STATUS_CONTINUE,CANTP_FLOW_BLOCK,CANTP_FLOW_TIMES,Handler->CanApi.tx);
                    }
                    
                }break;
                case CANTP_FLOWCONTROL_FRAME: //收到流控帧
                {
                    Handler->Rxmsg[i].flow = TRUE; 
                    Handler->Remote[i].block = *(msg+1);
                    Handler->Remote[i].status = *(msg) & 0x0F;
                    Handler->Remote[i].stmin = *(msg+2);
                }break;
                default: break;
            }
        }
        break;
    }
}

/*
 * 功能：接收
 * id：CAN帧ID
 * msg：数据
 * size：大小
*/
bool Cantp_Rx(Cantp_HandlerStruct* Handler, uint32_t* id, uint8_t* msg, uint32_t* size)
{
    for(int i = 0; i < CANTP_ID_MAX; i++)
    {
        if(Handler->Rxmsg[i].completed == TRUE)
        {
            id = &Handler->RxIdList[i];
            msg = Handler->Rxmsg[i].payload;
            size = &Handler->Rxmsg[i].size;
            return TRUE;
        }
    }
    return FALSE;
}

 void WEAK Cantp_RxCallback(Cantp_HandlerStruct* Handler, uint32_t id, uint8_t* msg, uint32_t size){}
 void WEAK Cantp_TxCallback(Cantp_HandlerStruct* Handler, uint32_t id){}

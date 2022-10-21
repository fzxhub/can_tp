#include "cantp.h"

/***************************************************************************************************************/
//Internal function
/***************************************************************************************************************/
/*
 * function: Single Send
 * id: id
 * msg: msg
 * size: size
 * Send: CAN HW Send
 * return: TRUE,FALSE
*/
static bool_t Cantp_Single(uint32_t id, uint8_t* msg, uint32_t size, Cantp_CanTx_t send)
{
    uint8_t temp_data[CANTP_FRAME_BYTE];
    uint8_t temp_size = 0;
    //if size more than 7, Can't handle
    if(size > CANTP_FRAME_BYTE-1) return FALSE;
    //if the cache is empty,Can't handle
    if(send == NULL)  return FALSE; 
    //Zero bytes said Single and length
    temp_data[0] = (CANTP_SINGLE_FRAME<<4) | size;
    //Copy data after the first byte
    memcpy(&temp_data[1], msg, size);
    //Padding invalid characters
    if(CANTP_FRAME_FILL == TRUE)
    {
        memset(&temp_data[size+1], CANTP_FRAME_VOID, CANTP_FRAME_BYTE - size - 1);
        temp_size = CANTP_FRAME_BYTE;
    }
    else temp_size = size + 1;
    
    //Send through CAN
    return send(id,temp_data,temp_size);
}
/*
 * function: First Send
 * id: id
 * msg: msg
 * size: size
 * Send: CAN HW Send
 * return: TRUE,FALSE
*/
static bool_t Cantp_First(uint32_t id, uint8_t* msg, uint32_t sizes, Cantp_CanTx_t send)
{
    //if size more than CANTP_MESSAGE_BYTE, Can't handle(This is the protocol maximum)
    //Note the cache size of this module
    if(sizes > CANTP_MESSAGE_BYTE) return FALSE;
    //if the cache is empty,Can't handle
    if(send == NULL)  return FALSE;
    //cache
    uint8_t temp[CANTP_FRAME_BYTE];
    //Zeroth and first bytes said Single and length
    temp[0] = (CANTP_FIRST_FRAME<<4) | (sizes>>8);
    temp[1] = sizes;
    //Copy data
    memcpy(&temp[2], msg, CANTP_FRAME_BYTE-2);
    //Send through CAN
    return send(id,temp,CANTP_FRAME_BYTE);
}
/*
 * function: Consecutive Send
 * id: id
 * msg: msg
 * size: size
 * Send: CAN HW Send
 * return: TRUE,FALSE
*/
static bool_t Cantp_Consecutive(uint32_t id, uint8_t* msg, uint32_t size, Cantp_CanTx_t send)
{
    //Consecutive count
    static uint8_t  time = 0x01;
    //The frame buffer
    uint8_t temp_data[CANTP_FRAME_BYTE];
    uint8_t temp_size = 0;
    //if size more than CANTP_MESSAGE_BYTE, Can't handle(This is the protocol maximum)
    //Note the cache size of this module
    if(size > CANTP_MESSAGE_BYTE) return FALSE;
    //if the cache is empty,Can't handle
    if(send == NULL)  return FALSE;
    
    //Zeroth bytes
    temp_data[0] = (CANTP_CONSECUTIVE_FRAME<<4) | (time&0x0f);
    //if size more than A frame
    if(size > CANTP_FRAME_BYTE-1)
    {
        memcpy(&temp_data[1], msg, CANTP_FRAME_BYTE-1);
        time++;
        temp_size = CANTP_FRAME_BYTE;
    }    
    else
    {
        memcpy(&temp_data[1], msg, size);
        //Populating invalid data
        if(CANTP_FRAME_FILL == TRUE)
        {
            memset(&temp_data[size+1], CANTP_FRAME_VOID, CANTP_FRAME_BYTE - size - 1);
            temp_size = CANTP_FRAME_BYTE;
        }
        else temp_size = size + 1;
        time = 0x01;
    }
    //Send through CAN
    return send(id,temp_data,temp_size);
}
/*
 * function: FlowControl Send
 * id: id
 * status: local status
 * block: block size
 * stmin: Minimum time interval
 * Send: CAN HW Send
 * return: TRUE,FALSE
*/
static bool_t Cantp_FlowControl(uint32_t id, Cantp_FlowStatus status, uint8_t block , uint8_t stmin, Cantp_CanTx_t send)
{
    uint8_t temp_data[CANTP_FRAME_BYTE];
    uint8_t temp_size = 0;
    //if the cache is empty,Can't handle
    if(send == NULL)  return FALSE;
    //Zeroth byte
    temp_data[0] = (CANTP_FLOWCONTROL_FRAME<<4) | (status);
    //first byte
    temp_data[1] = block;
    //second byte  0x00-0x7F:0-127ms  0xF1-0xF9:0.1-0.9ms
    if((stmin <= 0x7F) || (stmin >= 0xF1 && stmin <= 0xF9))
        temp_data[2] = stmin;
    //Populating invalid data
    if(CANTP_FRAME_FILL == TRUE)
    {
        memset(&temp_data[3], CANTP_FRAME_VOID, CANTP_FRAME_BYTE-3);
        temp_size = CANTP_FRAME_BYTE;
    }
    else  temp_size = 3;
    //Send through CAN   
    return send(id,temp_data,temp_size);
}

/***************************************************************************************************************/
//External interface(C)
/***************************************************************************************************************/
/*
 * function: Register
*/
void Cantp_CanRegister(Cantp_HandlerStruct* Handler, Cantp_CanTx_t tx, Cantp_CanRx_t rx)
{
    Handler->CanRx = rx;
    Handler->CanTx = tx;
    Handler->Local.stmin = 0; 
    Handler->Local.block = 8; 
}
void Cantp_CallbackRegister(Cantp_HandlerStruct* Handler, uint32_t ch, Cantp_TxCallback_t tx, Cantp_RxCallback_t rx)
{
    if(ch < CANTP_CALL_MAX)
    {
        Handler->TxCallback[ch]= tx;
        Handler->RxCallback[ch]= rx;
    }
}
void Cantp_CanidRegister(Cantp_HandlerStruct* Handler, uint32_t ch, uint32_t txid, uint32_t rxid)
{
    if(ch < CANTP_CALL_MAX)
    {
        Handler->TxIdList[ch] = txid;
        Handler->RxIdList[ch] = rxid;
    }
}
/*
 * function: Blocking Send
 * Handler: Handler
 * id: id
 * msg: msg
 * size: size
 * return: TRUE,FALSE
*/
bool_t Cantp_TxBlocking(Cantp_HandlerStruct* Handler, uint32_t id, uint8_t* msg, uint32_t size)
{
    uint32_t temp_size = 0;
    uint8_t temp_flow = 0;
    int32_t temp_index = -1;
    int32_t temp_times = 0;
    //Check if the ID is in the list
    for(int i = 0; i < CANTP_ID_MAX; i++)
    {
        if(id == Handler->TxIdList[i])
        {
            temp_index = i;
            break;
        }
    }
    if(size <= 7)
        return Cantp_Single(id,msg,size,Handler->CanTx); 
    else
    {
        Cantp_First(id,msg,size,Handler->CanTx);
        temp_size = temp_size + 6;
        temp_flow++;
        if(temp_index != -1)
        {
            Handler->TxState.wait = TRUE;
            while(1)
            {
                if(Handler->Rxmsg[temp_index].flow == TRUE)
                {
                    Handler->Rxmsg[temp_index].flow = FALSE;
                    Handler->TxState.wait = FALSE;
                    temp_times = 0;
                    goto SEND2;
                }
                else 
                {
                    DELAY_US();
                    temp_times++;
                    if(temp_times >= 1000) return FALSE;
                }
            }
        }        
    SEND2:
        while(1)
        {
            if(temp_flow%Handler->Local.block==0 && (temp_index != -1))
            {
                Handler->TxState.wait = TRUE;
                while(1)
                {
                    if(Handler->Rxmsg[temp_index].flow == TRUE)
                    {
                        Handler->Rxmsg[temp_index].flow = FALSE;
                        Handler->TxState.wait = FALSE;
                        temp_times = 0;
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
            Cantp_Consecutive(id,msg+temp_size,size-temp_size,Handler->CanTx);
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
 * function: Send
 * Handler: Handler
 * id: id
 * msg: msg
 * size: size
 * return: TRUE,FALSE
*/
bool_t Cantp_Tx(Cantp_HandlerStruct* Handler, uint32_t id, uint8_t* msg, uint32_t size)
{
    if(size > CANTP_MESSAGE_BYTE) return FALSE;
    Handler->TxState.id = id;
    memcpy(Handler->TxState.payload, msg, size);
    Handler->TxState.allsize = size;
    Handler->TxState.size = 0;
    Handler->TxState.frame = 0;
    Handler->TxState.runing = TRUE;
    return TRUE;
}
/*
 * function: Send Task
 * Handler: Handler
*/
void Cantp_TxTask(Cantp_HandlerStruct* Handler)
{
    int32_t temp_index = -1;
    if(Handler->TxState.runing == FALSE) return;
    //Check if the ID is in the list
    for(int i = 0; i < CANTP_ID_MAX; i++)
    {
        if(Handler->TxState.id == Handler->TxIdList[i])
        {
            temp_index = i;
            break;
        }
    } 
    //Send
    if(Handler->TxState.size == 0) //Start Send
    {
        if(Handler->TxState.allsize <= 7) //if less than 7,send thought Single
        {
            Cantp_Single(Handler->TxState.id, Handler->TxState.payload, Handler->TxState.allsize, Handler->CanTx);
            Handler->TxState.size = 0;
            Handler->TxState.frame = 0;
            Handler->TxState.runing = FALSE;
            for(int i = 0; i < CANTP_CALL_MAX; i++)
                Handler->TxCallback[i](Handler->TxState.id);
        }
        else
        {
            Cantp_First(Handler->TxState.id, Handler->TxState.payload, Handler->TxState.allsize, Handler->CanTx);
            Handler->TxState.size = Handler->TxState.size + 6;
            Handler->TxState.frame++;
        }
    }
    else
    {
        if((Handler->TxState.frame%Handler->Local.block == 0) || (Handler->TxState.frame == 1))
        {
            Handler->TxState.wait = TRUE;
            if((temp_index != -1) && (Handler->Rxmsg[temp_index].flow == TRUE)) 
            {
                Handler->Rxmsg[temp_index].flow = FALSE;
                Handler->TxState.wait = FALSE;
            }
            else if((temp_index != -1) && (Handler->Rxmsg[temp_index].flow == FALSE))
            {
                return;
            }
        }
        Cantp_Consecutive(Handler->TxState.id, Handler->TxState.payload+Handler->TxState.size,
        Handler->TxState.allsize-Handler->TxState.size, Handler->CanTx);
        Handler->TxState.frame++;
        if(Handler->TxState.allsize > Handler->TxState.size + 7)  
            Handler->TxState.size = Handler->TxState.size + 7;
        else
        {
            Handler->TxState.size = 0;
            Handler->TxState.frame = 0;
            Handler->TxState.runing = FALSE;
            for(int i = 0; i < CANTP_CALL_MAX; i++)
                Handler->TxCallback[i](Handler->TxState.id);
        } 
    }
}
/*
 * function: Receve Task
 * Handler: Handler
 * way: CALL_BACK,CALL_TASK
 * id: id
 * msg: msg
 * size: size
*/
void Cantp_RxTask(Cantp_HandlerStruct* Handler, Cantp_CallWay way,uint32_t id, uint8_t* msg, uint32_t size)
{
    uint8_t msg_temp[CANTP_FRAME_BYTE];
    //cache
    uint8_t* msg_c = NULL;
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
        if(Handler->CanRx(&id_c, msg_c, &size_c) == FALSE)
            return;
    }
    for(int i = 0; i < CANTP_ID_MAX; i++)
    {
        if(Handler->RxIdList[i] == id_c)
        {
            switch(((*msg_c)&0xF0)>>4)
            {
                case CANTP_SINGLE_FRAME:
                {
                    Handler->Rxmsg[i].size = (*msg_c)&0x0F;
                    memcpy(Handler->Rxmsg[i].payload,msg_c+1,Handler->Rxmsg[i].size);
                    Handler->Rxmsg[i].completed = TRUE;
                    Handler->Rxmsg[i].multi = FALSE;
                    for(int i = 0; i < CANTP_CALL_MAX; i++)
                        Handler->RxCallback[i](Handler->RxIdList[i],Handler->Rxmsg[i].payload,Handler->Rxmsg[i].size);                      
                }break;
                case CANTP_FIRST_FRAME:
                {
                    Handler->Rxmsg[i].allsize = (((*msg_c)&0x0F)<<8) + *(msg_c+1);
                    //There are only 6 valid slots left in the first frame
                    memcpy(Handler->Rxmsg[i].payload,msg_c+2,6);
                    Handler->Rxmsg[i].completed = FALSE;
                    Handler->Rxmsg[i].multi = TRUE;
                    Handler->Rxmsg[i].size = 6;
                    Handler->Rxmsg[i].frame = 1;
                    Cantp_FlowControl(Handler->TxIdList[i],CANTP_FLOW_STATUS_CONTINUE,Handler->Local.block,Handler->Local.stmin,
                    Handler->CanTx);
                }break;
                case CANTP_CONSECUTIVE_FRAME:
                {
                    if(Handler->Rxmsg[i].multi == FALSE) return;

                    if(Handler->Rxmsg[i].allsize >= Handler->Rxmsg[i].size + 7)
                    {
                        memcpy(Handler->Rxmsg[i].payload + Handler->Rxmsg[i].size, msg_c+1, 7);
                        Handler->Rxmsg[i].size += 7;
                        Handler->Rxmsg[i].frame++;
                    }
                    else
                    {
                        memcpy(Handler->Rxmsg[i].payload + Handler->Rxmsg[i].size, msg_c+1, 
                        Handler->Rxmsg[i].allsize - Handler->Rxmsg[i].size);
                        Handler->Rxmsg[i].size = Handler->Rxmsg[i].allsize;
                        Handler->Rxmsg[i].frame++;
                    }
                    if(Handler->Rxmsg[i].allsize == Handler->Rxmsg[i].size)
                    {
                        Handler->Rxmsg[i].completed = TRUE;
                        Handler->Rxmsg[i].multi = FALSE;
                        for(int i = 0; i < CANTP_CALL_MAX; i++)
                            Handler->RxCallback[i](Handler->RxIdList[i],Handler->Rxmsg[i].payload,Handler->Rxmsg[i].size);
                    }
                    //Each block needs to receive the flow control frame
                    if(Handler->Rxmsg[i].frame%Handler->Local.block == 0)
                    {
                        Cantp_FlowControl(Handler->TxIdList[i],CANTP_FLOW_STATUS_CONTINUE,Handler->Local.block,
                        Handler->Local.stmin,Handler->CanTx);
                    }
                    
                }break;
                case CANTP_FLOWCONTROL_FRAME: //receive the flow control frame
                {
                    if(Handler->TxState.wait != TRUE) return;
                    Handler->Rxmsg[i].flow = TRUE; 
                    Handler->Remote[i].block = *(msg_c+1);
                    Handler->Remote[i].status = (Cantp_FlowStatus)(*(msg_c) & 0x0F);
                    Handler->Remote[i].stmin = *(msg_c+2);
                }break;
                default: break;
            }
        }
        break;
    }
}
/*
 * function: Receve
 * Handler: Handler
 * id: id
 * msg: msg
 * size: size
*/
bool_t Cantp_Rx(Cantp_HandlerStruct* Handler, uint32_t* id, uint8_t** msg, uint32_t* size)
{
    for(int i = 0; i < CANTP_ID_MAX; i++)
    {
        if(Handler->Rxmsg[i].completed == TRUE)
        {
            *id = Handler->RxIdList[i];
            *msg = Handler->Rxmsg[i].payload;
            *size = Handler->Rxmsg[i].size;
            Handler->Rxmsg[i].completed = FALSE;
            return TRUE;
        }
    }
    return FALSE;
}

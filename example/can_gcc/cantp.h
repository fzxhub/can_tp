#ifndef __CANTP_H__
#define __CANTP_H__
/***************************************************************************************************************/
#include "string.h"
#include "stdio.h"
/***************************************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef FALSE
#define FALSE              0
#endif
#ifndef TRUE
#define TRUE               1
#endif
#ifndef NULL
#define NULL               0
#endif
#ifndef uint8_t
typedef unsigned char      uint8_t;
#endif
#ifndef uint16_t
typedef unsigned short     uint16_t;
#endif
#ifndef int32_t
typedef signed int         int32_t;
#endif
#ifndef uint32_t
typedef unsigned int       uint32_t;
#endif
typedef unsigned char      bool_t;
/***************************************************************************************************************/
//only used for TxBlocking
#define DELAY_MS(x)         ; 
//CANTP Block send Time interval (MS)
#define CANTP_FRAME_INTER  10 
//CANTP Block send Time flow (MS)
#define CANTP_WAIT_FLOW    500                     
/***************************************************************************************************************/
//CANTP frame most bytes
#define CANTP_FRAME_BYTE   8
//CANTP Send by most bytes for frame(CANTP_FRAME_BYTE)
#define CANTP_FRAME_FILL   TRUE
//CANTP Invalid content to fill
#define CANTP_FRAME_VOID   0XAA
//CANTP send Time flow (TX cycle)
#define CANTP_TXWAIT_FLOW  100    
//CANTP Protocol maximum bytes
#define CANTP_MESSAGE_BYTE 4095
//CANTP maximum bytes(cache)
#define CANTP_FLOW_BYTE    256
//CANTP Maximum number of ids managed by CAN
#define CANTP_ID_MAX       1
//CANTP Maximum number of Callback
#define CANTP_CALL_MAX     1
/***************************************************************************************************************/

typedef enum
{
    CANTP_SINGLE_FRAME       = 0x0,
    CANTP_FIRST_FRAME        = 0x1,
    CANTP_CONSECUTIVE_FRAME  = 0x2,
    CANTP_FLOWCONTROL_FRAME  = 0x3,
} Cantp_ProtocolControl;

typedef enum
{
    CANTP_FLOW_STATUS_CONTINUE = 0x0,  
    CANTP_FLOW_STATUS_WAIT     = 0x1, 
    CANTP_FLOW_STATUS_OVERFLOW = 0x2 
} Cantp_FlowStatus;

typedef enum
{
    CALL_BACK,
    CALL_TASK
} Cantp_CallWay;
/***************************************************************************************************************/
//CAN Send
typedef bool_t (*Cantp_CanTx_t)(uint32_t id, uint8_t* msg, uint32_t size);
//CAN Receive
typedef bool_t (*Cantp_CanRx_t)(uint32_t* id, uint8_t* msg, uint32_t* size);
//CAN Receive Call Back
typedef void (*Cantp_RxCallback_t)(uint32_t id, uint8_t* msg, uint32_t size);
//CAN Send Call Back
typedef void (*Cantp_TxCallback_t)(uint32_t id, bool_t results);

/***************************************************************************************************************/
//RX
typedef struct {
    uint8_t             payload[CANTP_FLOW_BYTE];   //Data
    uint32_t            size;                       //bytes size
    uint32_t            frame;                      //frame size
    uint32_t            allsize;                    //Expected total bytes  
    bool_t              completed;                  //completed  
    bool_t              multi;                      //Multiple frame tagging
    bool_t              flow;                       //rx flow frame
} Cantp_RxMsgStruct;
//TX
typedef struct {
    uint32_t            id;                         //tx id
    uint8_t             payload[CANTP_FLOW_BYTE];   //tx data
    uint32_t            allsize;                    //Expected total bytes
    uint32_t            size;                       //Number of send bytes
    uint32_t            frame;                      //Number of send frame
    bool_t              runing;                     //run flag
    bool_t              wait;                       //waitting flow flag
} Cantp_TxStateStruct;

typedef struct {
    uint8_t             block;      //block  
    uint8_t             stmin;      //frame Time interval between
    Cantp_FlowStatus    status;     //status 
} Cantp_AbilityStruct;
/***************************************************************************************************************/

typedef struct {
    uint32_t RxIdList[CANTP_ID_MAX];
    uint32_t TxIdList[CANTP_ID_MAX];  

    Cantp_AbilityStruct Remote[CANTP_ID_MAX];
    Cantp_RxMsgStruct   Rxmsg[CANTP_ID_MAX]; 

    Cantp_AbilityStruct Local;
    Cantp_TxStateStruct TxState;

    Cantp_CanTx_t CanTx;     
    Cantp_CanRx_t CanRx;  

    Cantp_TxCallback_t TxCallback[CANTP_CALL_MAX];
    Cantp_RxCallback_t RxCallback[CANTP_CALL_MAX];
} Cantp_HandlerStruct;

/***********************************************************************************************************************/
void   Cantp_CanRegister(Cantp_HandlerStruct* Handler, Cantp_CanTx_t tx, Cantp_CanRx_t rx);
void   Cantp_CallbackRegister(Cantp_HandlerStruct* Handler, uint32_t ch, Cantp_TxCallback_t tx, Cantp_RxCallback_t rx);
void   Cantp_CanidRegister(Cantp_HandlerStruct* Handler, uint32_t ch, uint32_t txid, uint32_t rxid);
void   Cantp_AbilityRegister(Cantp_HandlerStruct* Handler, uint8_t stmin, uint8_t block);
/***********************************************************************************************************************/
bool_t Cantp_TxBlocking(Cantp_HandlerStruct* Handler, uint32_t id, uint8_t* msg, uint32_t size);
bool_t Cantp_Tx(Cantp_HandlerStruct* Handler, uint32_t id, uint8_t* msg, uint32_t size);
void   Cantp_TxTask(Cantp_HandlerStruct* Handler);
bool_t Cantp_Rx(Cantp_HandlerStruct* Handler, uint32_t* id, uint8_t** msg, uint32_t* size);
void   Cantp_RxTask(Cantp_HandlerStruct* Handler, Cantp_CallWay way,uint32_t id, uint8_t* msg, uint32_t size);
/***********************************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif


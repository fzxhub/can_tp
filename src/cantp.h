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
#define WEAK                __attribute__ ((weak))  //弱函数定义
#define DELAY_US(x)         ;                       //延时定义(仅用于阻塞发送)
/***************************************************************************************************************/
//CANTP发送帧数据最大字节数
#define CANTP_FRAME_BYTE   8
//CANTP发送帧按最大帧字节发送(一帧数据不够CANTP_FRAME_BYTE，自动填)
#define CANTP_FRAME_FILL   TRUE
//CANTP发送帧按最大帧字节发送时填充的无效内容
#define CANTP_FRAME_VOID   0XAA
//CANTP发送消息最大字节数（CANTP协议规定）
#define CANTP_MESSAGE_BYTE 4095
//CANTP流控最大字节数(也就是本模块最大缓存数据)
#define CANTP_FLOW_BYTE    256
//CANTP单个CAN接口支持的管理ID最大个数
#define CANTP_ID_MAX      1
/***************************************************************************************************************/
//帧类型标志
typedef enum
{
    CANTP_SINGLE_FRAME       = 0x0,
    CANTP_FIRST_FRAME        = 0x1,
    CANTP_CONSECUTIVE_FRAME  = 0x2,
    CANTP_FLOWCONTROL_FRAME  = 0x3,
} Cantp_ProtocolControl;
//帧状态标志
typedef enum
{
    CANTP_FLOW_STATUS_CONTINUE = 0x0,  
    CANTP_FLOW_STATUS_WAIT     = 0x1, 
    CANTP_FLOW_STATUS_OVERFLOW = 0x2 
} Cantp_FlowStatus;
//调用方式
typedef enum
{
    CALL_BACK,
    CALL_TASK
} Cantp_CallWay;
/***************************************************************************************************************/
//CAN发送函数定义
typedef bool_t (*Cantp_CanTx)(uint32_t id, uint8_t* msg, uint32_t size);
//CAN接收函数定义
typedef bool_t (*Cantp_CanRx)(uint32_t* id, uint8_t* msg, uint32_t* size);
/***************************************************************************************************************/
//数据
typedef struct {
    uint8_t             payload[CANTP_FLOW_BYTE];//接收时，接收的数据
    uint32_t            size;       //接收时，已经接收字节数
    uint32_t            frame;      //接收时，接收帧数记录
    uint32_t            allsize;    //接收时，要接收的全部字节数   
    bool_t              completed;  //接收时，接收完成  
    bool_t              multi;      //接收时，是多帧数据
    bool_t              flow;       //接收时，收到流控帧 
} Cantp_RxMsgStruct;
//发送状态
typedef struct {
    uint32_t            id;         //发送时，CAN的发送ID
    uint8_t             payload[CANTP_FLOW_BYTE];   //发送时，发送数据
    uint32_t            allsize;    //发送时，需要发送的数据大小
    uint32_t            size;       //发送时，当前已经发送大小
    uint32_t            frame;      //发送时，已经发送帧数
    bool_t              runing;     //发送时，发送运行标志
    bool_t              wait;       //发送时，等待流控帧
} Cantp_TxStateStruct;
//CANTP传输能力
typedef struct {
    uint8_t             block;      //设备流控块数量   
    uint8_t             stmin;      //接收帧间隔最小值
    Cantp_FlowStatus    status;     //状态 
} Cantp_AbilityStruct;
/***************************************************************************************************************/
//CANTP注册
typedef struct {
    Cantp_CanTx tx;     //发送回调函数
    Cantp_CanRx rx;     //接收回调函数
} Cantp_CanApiStruct;
//CANTP句柄
typedef struct {
    //接收ID的列表
    uint32_t RxIdList[CANTP_ID_MAX];
    //发送ID的列表-与接收列表对应(主要用于流控发送)
    uint32_t TxIdList[CANTP_ID_MAX];
    //远程主机的接收能力   
    Cantp_AbilityStruct Remote[CANTP_ID_MAX];
    //接收消息
    Cantp_RxMsgStruct   Rxmsg[CANTP_ID_MAX];
    //CAN接收、发送接口函数
    Cantp_CanApiStruct CanApi;  
    //本地主机的接收能力  
    Cantp_AbilityStruct Local;
    //发送状态
    Cantp_TxStateStruct TxState;  
} Cantp_HandlerStruct;
/***************************************************************************************************************/
//注册
void   Cantp_Register(Cantp_HandlerStruct* Handler, Cantp_CanTx tx, Cantp_CanRx rx, uint32_t rxid, uint32_t txid);
//发送-阻塞
bool_t Cantp_TxBlocking(Cantp_HandlerStruct* Handler, uint32_t id, uint8_t* msg, uint32_t size);
//发送-非阻塞
bool_t Cantp_Tx(Cantp_HandlerStruct* Handler, uint32_t id, uint8_t* msg, uint32_t size);
//发送任务-非阻塞使用
void   Cantp_TxTask(Cantp_HandlerStruct* Handler);
//发送-回调函数
void   Cantp_TxCallback(Cantp_HandlerStruct* Handler, uint32_t id);
//接收
bool_t Cantp_Rx(Cantp_HandlerStruct* Handler, uint32_t* id, uint8_t** msg, uint32_t* size);
//接收-任务调度或者回调使用
void   Cantp_RxTask(Cantp_HandlerStruct* Handler, Cantp_CallWay way,uint32_t id, uint8_t* msg, uint32_t size);
//接收-回调函数
void   Cantp_RxCallback(Cantp_HandlerStruct* Handler, uint32_t id, uint8_t* msg, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif


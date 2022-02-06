#ifndef __ISOTP_H__
#define __ISOTP_H__

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned char      bool;

#define FALSE              0
#define TRUE               1
//#define NULL             0
#define WEAK                __attribute__ ((weak))  //GCC下若函数定义

//ISOTP发送帧数据最大字节数
#define ISOTP_FRAME_BYTE   8
//ISOTP发送帧按最大帧字节发送
#define ISOTP_FRAME_FILL   TRUE
//ISOTP发送帧按最大帧字节发送时填充的无效内容
#define ISOTP_FRAME_VOID   0XAA
//ISOTP发送消息最大字节数
#define ISOTP_MESSAGE_BYTE 4095
//ISOTP流控最大字节数(也就是本协议最大缓存数据)
#define ISOTP_FLOW_BYTE    256

//本机接收能力
//本机发送流控帧间隔块
#define ISOTP_FLOW_BLOCK     8
//本机接收时间间隔（MS）
#define ISOTP_FLOW_TIMES     1





typedef enum
{
    ISOTP_SINGLE_FRAME       = 0x0,
    ISOTP_FIRST_FRAME        = 0x1,
    ISOTP_CONSECUTIVE_FRAME  = 0x2,
    ISOTP_FLOWCONTROL_FRAME  = 0x3,
} Isotp_ProtocolControl;


typedef enum
{
    ISOTP_FLOW_STATUS_CONTINUE = 0x0,  
    ISOTP_FLOW_STATUS_WAIT     = 0x1, 
    ISOTP_FLOW_STATUS_OVERFLOW = 0x2 
} Isotp_FlowStatus;


typedef struct {
    uint32_t  id;    
    uint8_t   payload[ISOTP_FLOW_BYTE];
    uint32_t  size;      
    bool      completed;     
    bool      multi;  
} Isotp_Message;





typedef bool (*Isotp_CanSend)(uint32_t id, uint8_t* msg, uint32_t size);
typedef bool (*Isotp_CanRece)(uint32_t* id, uint8_t* msg, uint32_t* size);


typedef struct {
    // uint8_t             block;      //发送时，远端设备流控块数量   
    // uint8_t             stmin;      //发送时，远端接收间隔最小值
    // Isotp_FlowStatus    status;     //发送时，远端的状态 
    uint32_t            send_id;    //发送时，CAN的发送ID
    uint8_t*            data;       //发送时，发送数据指针
    uint32_t            size;       //发送时，需要发送的数据大小
    uint32_t            now_size;   //发送时，当前已经发送大小
    bool                runing;     //发送运行标志
    Isotp_CanSend       send;       //发送函数

} Isotp_Send_Struct;


Isotp_Send_Struct Send_Struct;


typedef struct {
    uint8_t             block;      //设备流控块数量   
    uint8_t             stmin;      //接收帧间隔最小值
    Isotp_FlowStatus    status;     //状态 
} Isotp_Ability_Struct;



bool Isotp_SingleSend(uint32_t id, uint8_t* data, uint32_t size, Isotp_CanSend CanSend);
bool Isotp_FirstSend(uint32_t id, uint8_t* data, uint32_t sizes, Isotp_CanSend CanSend);
bool Isotp_ConsecutiveSend(uint32_t id, uint8_t* data, uint32_t size, Isotp_CanSend CanSend);
bool Isotp_FlowControlSend(uint32_t id, Isotp_FlowStatus status, uint8_t block , uint8_t Stmin, Isotp_CanSend CanSend);


//同步发送，直到发送完成或者失败返回，否则阻塞当前TASK
bool Isotp_Send(uint32_t id, uint8_t* msg, uint32_t size, Isotp_CanSend CanSend);

//异步发送消息
bool Isotp_SendAsyn(uint32_t id, uint8_t* msg, uint32_t size, Isotp_CanSend CanSend);
//异步消息回调函数，上层实现
void Isotp_SendCall(bool status);
//异步发送任务，放入周期TASK中（执行周期注意和本端能力定义匹配，本机发送的流控消息匹配）
void Isotp_SendTask(float time);


#endif


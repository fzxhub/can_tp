#ifndef __CANTP_H__
#define __CANTP_H__

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned char      bool;

#define FALSE              0
#define TRUE               1
//#define NULL             0
#define WEAK                __attribute__ ((weak))  //GCC下若函数定义

//同步操作(发送接收)延时函数，使用操作系统的延时仅仅阻塞调用任务，裸机时阻塞整个CPU。
#define DELAY_US(x)         ;                       //延时定义

//CANTP发送帧数据最大字节数
#define CANTP_FRAME_BYTE   8
//CANTP发送帧按最大帧字节发送
#define CANTP_FRAME_FILL   TRUE
//CANTP发送帧按最大帧字节发送时填充的无效内容
#define CANTP_FRAME_VOID   0XAA
//CANTP发送消息最大字节数
#define CANTP_MESSAGE_BYTE 4095
//CANTP流控最大字节数(也就是本协议最大缓存数据)
#define CANTP_FLOW_BYTE    256

//本机接收能力
//本机发送流控帧间隔块
#define CANTP_FLOW_BLOCK     8
//本机接收时间间隔（MS）
#define CANTP_FLOW_TIMES     1


/****************************************************************************************/

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

//CAN发送函数定义
typedef bool (*Cantp_CanTx)(uint32_t id, uint8_t* msg, uint32_t size);
//CAN接收函数定义
typedef bool (*Cantp_CanRx)(uint32_t* id, uint8_t* msg, uint32_t* size);

/****************************************************************************************/

//数据
typedef struct {
    uint32_t  id;    
    uint8_t   payload[CANTP_FLOW_BYTE];
    uint32_t  size;      
    bool      completed;     
    bool      multi;  
} Cantp_Message;

//发送状态
typedef struct {
    uint32_t            send_id;    //发送时，CAN的发送ID
    uint8_t*            data;       //发送时，发送数据指针
    uint32_t            size;       //发送时，需要发送的数据大小
    uint32_t            now_size;   //发送时，当前已经发送大小
    bool                runing;     //发送运行标志
} Cantp_TxStateStruct;
//发送状态实例
Cantp_TxStateStruct Cantp_TxState;

//CANTP传输能力
typedef struct {
    uint8_t             block;      //设备流控块数量   
    uint8_t             stmin;      //接收帧间隔最小值
    Cantp_FlowStatus    status;     //状态 
} Cantp_AbilityStruct;
//CANTP传输能力实例
Cantp_AbilityStruct Cantp_AbilityRemote;
Cantp_AbilityStruct Cantp_AbilityLocal;

//CANTP注册
typedef struct {
    Cantp_CanTx tx;
    Cantp_CanRx rx;
} Cantp_CanApiStruct;
//CANTP注册实例
Cantp_CanApiStruct Cantp_CanApi;







void Cantp_SendCall(bool status);
void Cantp_register(Cantp_CanTx tx, Cantp_CanRx rx);
bool Cantp_BlockingTx(uint32_t id, uint8_t* msg, uint32_t size);
bool Cantp_Tx(uint32_t id, uint8_t* msg, uint32_t size);
void Cantp_TxTask(float time);



#endif


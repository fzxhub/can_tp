# CAN-TP层协议

## 说明

这是一个C编写的基于CAN的ISO-TP(ISO15765-2)协议以及测试。框架分层严格、与平台无关、可配置、可裁剪。
也可以移植到其他硬件通信之上。

## 接口函数说明

该模块依赖底层CAN的接收发送函数，接收发送函数定义如下：

```c
//CAN发送函数定义
typedef bool (*Cantp_CanTx)(uint32_t id, uint8_t* msg, uint32_t size);
//CAN接收函数定义
typedef bool (*Cantp_CanRx)(uint32_t* id, uint8_t* msg, uint32_t* size);
```

## 使用步骤

#### 与底层对接
1. 定义一个Cantp_HandlerStruct的结构提用于本模块通信管理。例如定义为Cantp_HandlerStruct Handler;
2. 将CAN底层的发送接收函数封装成如上格式。
3. 使用Cantp_register将CAN发送函数、接收函数、发送ID、接收ID传入该函数。Cantp_register配置参数较少，如需更多配置直接给该句柄赋值即可，
4. 如果使用的是接收回调的方式，在底层回调中调用Cantp_RxTask(Handler, CALL_BACK, id, msg, size);
5. 非接收回调与底层对接，需要轮询Cantp_RxTask(Handler, CALL_TASK, id, msg, size);
#### 使用该模块
##### 发送
1. 使用阻塞发送调用Cantp_TxBlocking函数。
2. 非阻塞发送使用Cantp_Tx函数，使用该函数时，周期调用Cantp_TxTask。
##### 接收
1. 使用回调方式：Cantp_RxCallback，收到完整数据回调该函数。
2. 轮询方式：周期调用Cantp_Rx，收到完整消息返回真。


# CAN-TP层协议

## 说明

这是一个C编写的基于CAN的ISO-TP(ISO15765-2)协议以及测试。框架分层严格、与平台无关、可配置、可裁剪。也可以移植到其他硬件通信之上。
学习了CAN TP、UDS协议后，觉得该协议在实现时可以解耦合，看了一些开源的协议，觉得自己可以来做一下。

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

## 存在问题

1. 本模块依赖CAN底层的收发实现。在发送部分单一帧发送后还没有接口来判定是否发送成功，应发送成功后再发送下一帧才保险，后续完善。
2. 非阻塞发送中等待流控帧还没有设置超时时间，现在是死等，后续需要完善。
3. 发送中收到流控帧，就继续发送下一块，还没有对流控帧进行解析。
4. 收取连续帧时应对连续帧的序列进行判定。防止有丢帧情况，现在暂时无，后续完善。

## 实例说明

1. 实例一直接使用gcc编译后即可运行测试，由于无底层的CAN支持，测试较少，没有搭建其他特殊的测试环境
2. 在ESP32上测试，使用Arduino框架搭建了CAN的收发来测试。


# CAN-TP层协议

## 说明

这是一个C编写的基于CAN的ISO-TP(ISO15765-2)协议以及测试。框架分层严格、与平台无关、可配置、可裁剪。也可以移植到其他硬件通信之上。
学习了CAN TP、UDS协议后，觉得该协议在实现时可以解耦合。

## 更新说明

2022.10.21：

	- 更改回调函数（由弱函数改为注册函数），不依赖编译器并且支持多回调函数。

2022.10.22：

	- 发送一帧发送后判断底层接口返回值，返回真再发送下一帧，否则表示发送失败。
	- 非阻塞发送中等待流控帧设置超时时间。
	- 发送中收到流控帧（继续状态），就继续发送下一流控数据。

## 还存在问题

1. 收取连续帧时应对连续帧的序列进行判定。防止有丢帧情况，现在暂时无，后续完善。

## 特点

1. 在该模块每个句柄中发送和接收都有自己独立的缓存区（除了阻塞发送），只有数据完成后才通知上层应用。缓存区大小可自定义。
2. 该模块维护着发送ID和接收ID列表，只有符合该表的接受ID才进行接收，并自动拉取发送ID表中的ID进行流控回复，不在发送ID列表的ID
数据也可发送，但是不会进行流控管理。
3. 如果要管理多个CAN接口，只增加句柄即可开辟独立空间和接口进行管理和通信

## 接口函数说明

该模块依赖底层CAN的接收发送函数，接收发送函数定义如下：

```c
//CAN发送函数定义
typedef bool (*Cantp_CanTx)(uint32_t id, uint8_t* msg, uint32_t size);
//CAN接收函数定义
typedef bool (*Cantp_CanRx)(uint32_t* id, uint8_t* msg, uint32_t* size);
```

该模块与上层通信接口如下，需要上层提供处理函数然后注册到CANTP层：

```c
//CAN Receive Call Back
typedef void (*Cantp_RxCallback_t)(uint32_t id, uint8_t* msg, uint32_t size);
//CAN Send Call Back
typedef void (*Cantp_TxCallback_t)(uint32_t id, bool_t results);
```

## 使用步骤

#### 与底层对接
1. 定义一个Cantp_HandlerStruct的结构提用于本模块通信管理。例如定义为Cantp_HandlerStruct Handler;
2. 将CAN底层的发送接收函数封装成如上格式。
3. 使用Cantp_register类函数将CAN发送函数、接收函数、发送ID、接收ID、回调函数传入该函数。Cantp_register配置参数较少，如需更多配置直接给该句柄赋值即可，
4. 如果使用的是接收回调的方式，在底层回调中调用Cantp_RxTask(Handler, CALL_BACK, id, msg, size);
5. 非接收回调与底层对接，需要轮询Cantp_RxTask(Handler, CALL_TASK, id, msg, size);
#### 使用该模块
##### 发送
1. 使用阻塞发送调用Cantp_TxBlocking函数。
2. 非阻塞发送使用Cantp_Tx函数，使用该函数时，周期调用Cantp_TxTask。
##### 接收
1. 使用回调方式：Cantp_RxCallback，收到完整数据回调该函数。
2. 轮询方式：周期调用Cantp_Rx，收到完整消息返回真。

## 实例说明

1. 实例一直接使用gcc编译后即可运行测试，由于无底层的CAN支持，测试较少，没有搭建其他特殊的测试环境
2. 在ESP32上测试，使用Arduino框架搭建了CAN的收发来测试。


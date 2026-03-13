# MQTT（C）— 面向 STM32 的报文打包/解析库（不含网络层）

这是一个基于 MQTT 协议规范实现的 **报文打包/解析** 项目，目标是作为“可移植库”集成到 STM32 等资源受限 MCU 工程中。

> 说明：项目正在从“协议验证版”重构为可移植库；master 分支内容会持续调整，QoS2/注释等会在重构过程中逐步统一。

## 特性 / Scope
- ✅ 不包含 socket / AT 通信层（网络收发由用户工程负责）
- ✅ 报文打包：CONNECT、SUBSCRIBE、UNSUBSCRIBE、PUBLISH、PINGREQ、DISCONNECT
- ✅ 报文解析：CONNACK、SUBACK、UNSUBACK、PINGRESP；解析服务端下发 PUBLISH（提取 topic/payload）
- ✅ QoS 支持：
  - QoS0：PUBLISH
  - QoS1：PUBACK
  - QoS2：PUBREC / PUBREL / PUBCOMP（流程接口已提供，重构中）

## 目录结构
- `inc/`：对外头文件（API、结构体、宏）
- `src/`：实现（报文打包/解析）
- `examples/`：示例（展示如何调用 API 生成/解析报文）

## 快速开始（生成一条报文）
库本身只负责“生成/解析报文”，用户需要自己提供发送/接收通道（UART/4G 模组/ESP32/Wi-Fi 模组等）。

典型使用流程：
1. `MQTT_Init()` 初始化参数（ClientID/User/Password/Will 等）
2. 调用 `MQTT_CONNECT()` / `MQTT_SUBSCRIBE()` / `MQTT_PUBLISH()` 生成报文（写入 `m->buff`）
3. 将 `m->buff[0..m->length.Totallength)` 通过你的网络层发送出去
4. 收到服务器回包后，调用 `MQTT_CONNACK()` / `MQTT_SUBACK()` / `MQTT_PINGRESP()` 等解析

> 注意：接收侧需要你提供“一帧完整 MQTT 报文”的 buffer（例如从串口/模块回包中整理出来）。

## 示例（伪代码）
```c
#include "MQTT.h"

MQTT_TCB m;

MQTT_config_t cfg = {
  .ClientID = "dev001",
  .UserName = "user",
  .Passward = "pwd",
  .WillEnable = 0,
  .WillTopic = "",
  .WillData = "",
  .WillRetain = 0,
  .WillQoS = 0,
  .CleanSession = 1
};

int main(void)
{
  MQTT_Init(&m, &cfg);

  // 1) 生成 CONNECT 报文
  int n = MQTT_CONNECT(&m, 60);
  // send(m.buff, n);

  // 2) 订阅主题
  MQTT_SUBSCRIBE(&m, "test/topic", 0);
  // send(m.buff, m.length.Totallength);

  // 3) 发布
  MQTT_PUBLISH(&m, 0, 0, 0, "test/topic", (u8*)"hello", 5);
  // send(m.buff, m.length.Totallength);
}
```

## STM32 移植建议
- 建议将 `inc/` + `src/` 直接加入 STM32 工程
- 建议由上层提供：
  - `mqtt_send(uint8_t* buf, uint16_t len)`：发送接口
  - `mqtt_recv(uint8_t* buf, uint16_t maxlen)`：接收接口（或 ringbuffer + 帧解析）
- 若需要支持流式解析（粘包/拆包），建议在上层实现 ringbuffer，再将完整帧交给本库解析函数

## TODO（重构方向）
- [ ] 统一 QoS2 相关接口/注释与行为（inflight 状态管理、超时重传策略）
- [ ] 补充更清���的 examples：完整演示 CONNECT→SUB→PUBLISH→PING
- [ ] 增加错误码与参数校验（避免隐式行为）

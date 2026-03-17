# MQTT Client Core (C) — 可移植轻量内核（不含网络层）

一个面向 MCU/嵌入式场景的 **MQTT 客户端内核**：负责报文打包/解析 + keepalive + QoS1 重发；网络收发由上层通过回调注入（socket/AT/UART/lwIP 均可适配）。

## 亮点
- **协议栈与网络层解耦**：通过 `on_send(data,len)->int` 输出报文，通过 `MQTT_InputBytes()` 输入接收数据
- **非阻塞主循环驱动**：`MQTT_Process()` 统一处理 keepalive、超时、QoS1 PUBACK 重传
- **QoS1 可靠机制**：PUBACK 超时重发（设置 DUP=1），支持最大重试次数与超时错误返回
- **工程化发送语义**：`on_send` 返回实际发送长度；仅在发送成功时更新 `last_tx_ms` / 清 pending，避免短写/失败导致状态错乱

> 当前限制：单 inflight QoS1（同一时刻仅允许一个 QoS1 PUBLISH 等待 PUBACK）。

## 快速开始
### 1) 初始化与回调
```c
MQTT_TCB m;
MQTT_Init(&m, &cfg);
MQTT_SetAllOnCb_same(&m, &(MQTT_Callbacks){
  .on_send = my_on_send,  // int my_on_send(void* ctx, const uint8_t* data, uint16_t len)
  .on_connack = my_on_connack,
  .on_suback = my_on_suback,
  .on_puback = my_on_puback,
  .on_pingresp = my_on_pingresp,
});
MQTT_SetNowMs(&m, my_now_ms, &clock);
Mqtt_SetKeepalive(&m, 10000, 2000);
```

### 2) 打包发送（示例）
```c
mqtt_pack_connect(&m, m.io.connect_buf, MQTT_CONNECT_BUF_SIZE, 10000);
mqtt_emit_send(&m);

mqtt_pack_subscribe(&m, m.io.subscribe_buf, MQTT_SUBSCRIBE_BUF_SIZE, "TOPIC001", 0);
mqtt_emit_send(&m);
```

### 3) 接收与驱动
```c
// 收到网络数据后喂给库（支持分片/粘包，内部会组帧解析）
MQTT_InputBytes(&m, rx_data, rx_len);

// 在主循环周期性调用
int rc = MQTT_Process(&m);
if(rc < 0) {
  // TIMEOUT / MALFORMED / ...
}
```

## 验证
- QoS1 重发：故意延迟/丢弃 PUBACK，观察库自动重发 PUBLISH（DUP=1）
- Keepalive：故意不返回 PINGRESP，触发 `MQTT_ERR_TIMEOUT`
- Send 背压：让 `on_send` 返回 `-EAGAIN`，确认 pending 不丢，下一轮仍可继续发送


## 目录
- `inc/`：对外头文件（API、结构体、错误码）
- `src/`：实现（pack/parse/process）
- `examples/`：示例（app_demo：CONNECT→SUB→PUBLISH→PUBACK→PING）

## 后续计划（非必须）
- 重连后自动重订阅（re-subscribe）
- QoS2 流程统一（状态机/超时策略）
- 增加更多 examples / CI

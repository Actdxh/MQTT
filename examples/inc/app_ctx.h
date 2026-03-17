#ifndef APP_CTX_H
#define APP_CTX_H
#include "mqtt_type.h"


typedef enum{
    APP_EVT_NONE = 0,

    APP_EVT_CONNACK_OK,
    APP_EVT_CONNACK_FAIL,

    APP_EVT_SUBACK_OK,
    APP_EVT_SUBACK_FAIL,

    APP_EVT_UNSUBACK_OK,
    APP_EVT_UNSUBACK_FAIL,

    APP_EVT_PUBACK_OK,
    APP_EVT_PUBACK_FAIL,

    APP_EVT_PINGRESP,

    APP_EVT_PUBLISH_RX,
} app_event_type_t;

typedef struct{
    app_event_type_t type;
    uint16_t pid;
}app_evt_t;


#define APP_EVT_Q_SIZE 16
typedef struct{
    volatile uint8_t head;
    volatile uint8_t tail;
    app_evt_t queue[APP_EVT_Q_SIZE];
}app_evt_queue_t;

inline void app_ctx_init(app_evt_queue_t* ctx);
inline int app_evt_push(app_evt_queue_t* ctx, app_evt_t e);
inline int app_evt_pop(app_evt_queue_t* ctx, app_evt_t* e);





#endif // !APP_CTX_H

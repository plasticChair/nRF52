
#ifndef BLE_EINK_H__
#define BLE_EINK_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EINK_UUID_BASE        {0xc4, 0x61, 0x6b, 0x13, 0xda, 0xa2, 0x45, 0x7c, \
                              0x94, 0xd3, 0xa7, 0x88, 0x93, 0x98, 0x81, 0x22}
#define EINK_UUID_SERVICE     0x1300
#define EINK_UUID_DISPTXT 0x1331
#define EINK_UUID_DEMOSCR 0x1332

typedef struct ble_eink_s ble_eink_t;

typedef void (*ble_eink_disp_write_handler_t) (uint16_t conn_handle, ble_eink_t * p_eink, uint8_t len, uint8_t *data);
typedef void (*ble_eink_demo_write_handler_t) (uint16_t conn_handle, ble_eink_t * p_eink, uint8_t data);

typedef struct
{
    ble_eink_disp_write_handler_t disptxt_write_handler;
    ble_eink_demo_write_handler_t demoscr_write_handler;
    uint8_t *disptxt_buffer; /**< Display text buffer */
} ble_eink_init_t;

struct ble_eink_s
{
    uint16_t                    service_handle;      /**< Handle of eInk Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t    disptxt_char_handles;    /**< Handles related to the eink display text */
    ble_gatts_char_handles_t    demoscr_char_handles; /**< Handles related to the shown demo screen */
    uint8_t                     uuid_type;           /**< UUID type for the LED Button Service. */
    ble_eink_disp_write_handler_t disptxt_write_handler;
    ble_eink_demo_write_handler_t demoscr_write_handler;
};


uint32_t einksvc_init(ble_eink_t * p_eink, const ble_eink_init_t * p_eink_init);

void ble_eink_on_ble_evt(ble_eink_t * p_lbs, ble_evt_t * p_ble_evt);


#ifdef __cplusplus
}
#endif

#endif // BLE_LBS_H__

/** @} */

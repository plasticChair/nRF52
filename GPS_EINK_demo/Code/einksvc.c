/**
 *
 */

#include "sdk_common.h"
#include "einksvc.h"
#include "ble_srv_common.h"

/**@brief Function for handling the Write event.
 *
 * @param[in] p_eink     eInk Service structure.
 * @param[in] p_ble_evt  Event received from the BLE stack.
 */
static void on_write(ble_eink_t * p_eink, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if ((p_evt_write->handle == p_eink->disptxt_char_handles.value_handle) &&
        (p_eink->disptxt_write_handler != NULL))
    {
        p_eink->disptxt_write_handler(p_ble_evt->evt.gap_evt.conn_handle, p_eink, 
                p_evt_write->len, p_evt_write->data);
    }
    else if((p_evt_write->handle == p_eink->demoscr_char_handles.value_handle) &&
        (p_evt_write->len == 1) &&
        (p_eink->demoscr_write_handler != NULL))
    {
        p_eink->demoscr_write_handler(p_ble_evt->evt.gap_evt.conn_handle, p_eink, p_evt_write->data[0]);
    }
}


void ble_eink_on_ble_evt(ble_eink_t * p_eink, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GATTS_EVT_WRITE:
            on_write(p_eink, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

static uint32_t demoscr_char_add(ble_eink_t * p_eink, const ble_eink_init_t * p_eink_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_eink->uuid_type;
    ble_uuid.uuid = EINK_UUID_DEMOSCR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = 1;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = 1;
    attr_char_value.p_value      = NULL;

    return sd_ble_gatts_characteristic_add(p_eink->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_eink->demoscr_char_handles);
}

static uint32_t disptxt_char_add(ble_eink_t * p_eink, const ble_eink_init_t * p_eink_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_eink->uuid_type;
    ble_uuid.uuid = EINK_UUID_DISPTXT;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = 16;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = 16;
    attr_char_value.p_value      = p_eink_init->disptxt_buffer;

    return sd_ble_gatts_characteristic_add(p_eink->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_eink->disptxt_char_handles);
}

uint32_t einksvc_init(ble_eink_t * p_eink, const ble_eink_init_t * p_eink_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure.
    p_eink->disptxt_write_handler = p_eink_init->disptxt_write_handler;
    p_eink->demoscr_write_handler = p_eink_init->demoscr_write_handler;

    // Add service.
    ble_uuid128_t base_uuid = {EINK_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_eink->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_eink->uuid_type;
    ble_uuid.uuid = EINK_UUID_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_eink->service_handle);
    VERIFY_SUCCESS(err_code);

    // Add characteristics.
    err_code = disptxt_char_add(p_eink, p_eink_init);
    VERIFY_SUCCESS(err_code);
    err_code = demoscr_char_add(p_eink, p_eink_init);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}


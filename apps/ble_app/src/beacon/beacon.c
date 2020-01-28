#include <string.h>

#include "host/ble_hs.h"

#include "beacon_conf.h"
#include "beacon_priv.h"

#define GAP_DISCOVERY_FLAG_LE              0x01
#define GAP_DISCOVERY_FLAG_BREDR           0x02
#define GAP_DISCOVERY_FLAG_LIMITED         0x04
#define GAP_DISCOVERY_FLAG_LE_ACTIVE_SCAN  0x08
#define GAP_DISCOVERY_FLAG_LE_OBSERVE      0x10

uint8_t uuid128[16];
uint16_t major;
uint16_t minor;
int8_t tx_power;

static
void ble_app_set_addr()
{
    ble_addr_t addr;
    int rc;

    rc = ble_hs_id_gen_rnd(1, &addr);
    assert(rc == 0);

    rc = ble_hs_id_set_rnd(addr.val);
    assert(rc == 0);
}

static void ble_app_advertise()
{
    struct ble_gap_adv_params adv_params;
    int rc;

    /* Major version=2; minor version=10. */
    rc = ble_ibeacon_set_adv_data(uuid128, major, minor, tx_power);
    assert(rc == 0);

    /* Begin advertising. */
    adv_params = (struct ble_gap_adv_params){ 0 };
    adv_params.itvl_min = 1000 * 1000 / BLE_HCI_ADV_ITVL;
    adv_params.itvl_max = adv_params.itvl_min;
    rc = ble_gap_adv_start(BLE_OWN_ADDR_RANDOM, NULL, BLE_HS_FOREVER,
                       &adv_params, NULL, NULL);
    assert(rc == 0);
}

static int gap_event_handler(struct ble_gap_event *event, void *arg)
{
    if (event->type == BLE_GAP_EVENT_DISC)
    {
        struct ble_gap_disc_desc *disc = &(event->disc);
        //printf("DISC: ");
        int i;
        //for(i = 5; i > 0; i--)
        //{
        //    printf("%02x:", disc->addr.val[i]);
        //}
        //printf("%02x %ddBm\n", disc->addr.val[i], disc->rssi);
        //printf(" -> Data: ");
        //for(i = 0; i < disc->length_data - 1; i++)
        //{
        //    printf("%02x ", disc->data[i]);
        //}
        //printf("%02x (%d bytes)\n", disc->data[i], disc->length_data);
        if (disc->length_data != 30)
            return 0;
        struct ble_hs_adv_fields adv_fields = { 0 };
        ble_hs_adv_parse_fields(&adv_fields, disc->data, disc->length_data);
        if (adv_fields.flags != (GAP_DISCOVERY_FLAG_LIMITED | GAP_DISCOVERY_FLAG_BREDR))
            return 0;
        if (adv_fields.mfg_data == NULL || adv_fields.mfg_data_len != 25)
            return 0;
        if (adv_fields.mfg_data[0] != 0x4C
            || adv_fields.mfg_data[1] != 0x00
            || adv_fields.mfg_data[2] != 0x02
            || adv_fields.mfg_data[3] != 0x15)
            return 0;
        printf("{\"advertising\":\"iBeacon\",\"rssi\":%d,\"data\":{\"uuid\":\"", disc->rssi);
        for(i = 0; i < 16; i++)
        {
            printf("%02x", adv_fields.mfg_data[4 + i]);
        }
        printf("\",\"major\":\"%#02x%02x\",\"minor\":\"%#02x%02x\",\"tx_power\":%d}}\n", adv_fields.mfg_data[20], adv_fields.mfg_data[21], adv_fields.mfg_data[22], adv_fields.mfg_data[23], (int8_t)adv_fields.mfg_data[24]);
    }
    return 0;
}

static void ble_app_discover()
{
    struct ble_gap_disc_params disc_params = { 0 };
    int rc = ble_gap_disc(BLE_OWN_ADDR_RANDOM, BLE_HS_FOREVER, &disc_params, gap_event_handler, NULL);
    assert(rc == 0);
}

static void ble_app_on_sync(void)
{
    /* Generate a non-resolvable private address. */
    ble_app_set_addr();

    /* Advertise indefinitely. */
    ble_app_advertise();

    ble_app_discover();
}

void beacon_init()
{
    beacon_conf_init();

    ble_hs_cfg.sync_cb = ble_app_on_sync;
}

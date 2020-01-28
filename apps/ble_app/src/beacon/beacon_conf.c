#include "sysinit/sysinit.h"
#include "os/os.h"
#include "config/config.h"
#include "beacon_priv.h"

static int
beacon_conf_set(int argc, char **argv, char *val)
{
    if (argc == 1)
    {
        if (!strcmp(argv[0], "uuid"))
        {
            if (strlen(val) != 32)
            {
                return OS_INVALID_PARM;
            }
            for (int i = 0; i < 32; i++)
            {
                uint8_t byte = val[i];
                if (byte >= '0' && byte <= '9') byte = byte - '0';
                else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
                else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
                else return OS_INVALID_PARM;
                // shift 4 to make space for new digit, and add the 4 bits of the new digit
                uuid128[i / 2] = (uuid128[i / 2] << 4) | (byte & 0xF);
            }
            return 0;
        }
        if (!strcmp(argv[0], "major"))
        {
            CONF_VALUE_SET(val, CONF_INT16, major);
            return 0;
        }
        if (!strcmp(argv[0], "minor"))
        {
            CONF_VALUE_SET(val, CONF_INT16, minor);
            return 0;
        }
        if (!strcmp(argv[0], "tx_power"))
        {
            CONF_VALUE_SET(val, CONF_INT8, tx_power);
            return 0;
        }
    }
    return OS_ENOENT;
}

static int
beacon_conf_export(void (*func)(char *name, char *val),
  enum conf_export_tgt tgt)
{
    char buf[37];

    int offset = 0;
    for (int i = 0; i < sizeof(uuid128); i++)
    {
        snprintf(buf + offset, 3, "%02x", uuid128[i]);
        offset += 2;
        if (tgt == CONF_EXPORT_PERSIST)
            continue;
        if (i == 3 || i == 5 || i == 7 || i == 9)
            buf[offset++] = '-';       
    }

    func("beacon/uuid", buf);

    snprintf(buf, 6, "%u", major);
    func("beacon/major", buf);

    snprintf(buf, 6, "%u", minor);
    func("beacon/minor", buf);
   
    if (tgt == CONF_EXPORT_PERSIST)
        conf_str_from_value(CONF_INT8, &tx_power, buf, 5);
    else
        snprintf(buf, 9, "%d dBm", tx_power);
    func("beacon/tx_power", buf);

    return 0;
}

static struct conf_handler beacon_conf_handler = {
    .ch_name = "beacon",
    .ch_get = NULL,
    .ch_set = beacon_conf_set,
    .ch_commit = NULL,
    .ch_export = beacon_conf_export,
};

void beacon_conf_init(void)
{
    int rc;

    rc = conf_register(&beacon_conf_handler);
    SYSINIT_PANIC_ASSERT_MSG(rc == 0,
                             "Failed to register beacon conf");

    conf_load();
};

#include "ctc_def.h"
#include "msg_util.h"
#include "com_def.h"
#include <string.h>

int ctc_send_msg(const uint16_t event, const Msg_direction* dst_dir,\
                 const void *data, const uint16_t len, const uint8_t is_need_ack)

{
    if (len > MAX_BODY_LEN)
    {
        // TRACE_ERR
        return FAILED;
    }

    Msg_pkg msg;
    (void)memset(&msg, 0x00, sizeof(msg));
    package_new_ptp_msg(&msg, event, dst_dir, data, len, is_need_ack);

    if (is_cross_dev(dst_dir->shelf, dst_dir->card))
    {
        if (len > MAX_EXT_MSG_LEN)
        {
            // TRACE_ERR
            return FAILED;
        }

        // TRACE_INFO
        return transmit_msg(&msg, sizeof(msg));
    }

    // TRACE_INFO
    return send_msg_to_local(dst_dir->pid, &msg, sizeof(msg));
}

int ctc_send_old_msg(const uint16_t event, const Msg_direction* dst_dir, const void* data, \
                     const uint16_t len, const uint8_t cmd_type ,const uint8_t is_need_ack)
{
    if (len > MAX_BODY_LEN)
    {
        // TRACE_ERR
        return FAILED;
    }

    Msg_pkg msg;
    (void)memset(&msg, 0x0, sizeof(msg));
    package_old_ptp_msg(&msg, event, dst_dir, data, len, cmd_type, is_need_ack);

    if (len > MAX_EXT_MSG_LEN)
    {
        // TRACE_ERR
        return FAILED;
    }

    // TRACE_INFO
    return transmit_msg(&msg, sizeof(msg));
}

int ctc_send_sync_msg(const uint16_t _event, const Msg_direction* _dst_dir, \
                      const void* _data, const uint16_t _len)
{
    return ctc_send_msg(_event, _dst_dir, _data, _len, NEED_ACK);
}

int ctc_send_asyn_msg(const uint16_t _event, const Msg_direction* _dst_dir, \
                      const void* _data, const uint16_t _len)
{
    return ctc_send_msg(_event, _dst_dir, _data, _len, NO_ACK);
}

int ctc_send_inter_msg(const uint16_t _event, const uint8_t _pid, const void* _data, \
                       const uint16_t _len, const uint8_t _ack_flag)
{
    Msg_direction dst;
    dst.shelf = get_self_shelf_id();
    dst.card = get_self_card_id();
    dst.pid = _pid;

    return ctc_send_msg(_event, &dst, _data, _len, _ack_flag);
}
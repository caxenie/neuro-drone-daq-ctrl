#ifndef  _MAVLINK_CONN_MAVLINK_PARSE_CHAR_H_
#define  _MAVLINK_CONN_MAVLINK_PARSE_CHAR_H_

#include "mavlink_conn_types.h"

// modified:
//      - pointer to message buffer
//      - pointer to status buffer
//      - MAVLINK_MESSAGE_CRC macro
//      - MAVLINK_MESSAGE_LENGTH macro

uint8_t ml_conn_mavlink_parse_char(ml_conn_t* mlc, uint8_t c, mavlink_message_t* r_message, mavlink_status_t* r_mavlink_status)
{
        /*
      default message crc function. You can override this per-system to
      put this data in a different memory segment
    */
#if MAVLINK_CRC_EXTRA
#ifndef ML_CONN_MAVLINK_MESSAGE_CRC
    static const uint8_t mavlink_message_crcs[256] = MAVLINK_MESSAGE_CRCS;
#define ML_CONN_MAVLINK_MESSAGE_CRC(msgid) mavlink_message_crcs[msgid]
#endif
#endif

/* Enable this option to check the length of each message.
 This allows invalid messages to be caught much sooner. Use if the transmission
 medium is prone to missing (or extra) characters (e.g. a radio that fades in
 and out). Only use if the channel will only contain messages types listed in
 the headers.
*/
#if MAVLINK_CHECK_MESSAGE_LENGTH
#ifndef ML_CONN_MAVLINK_MESSAGE_LENGTH
    static const uint8_t mavlink_message_lengths[256] = MAVLINK_MESSAGE_LENGTHS;
#define ML_CONN_MAVLINK_MESSAGE_LENGTH(msgid) mavlink_message_lengths[msgid]
#endif
#endif

    mavlink_message_t* rxmsg = &mlc->msg_buf;
    mavlink_status_t* status = &mlc->status_buf;

//	mavlink_message_t* rxmsg = mavlink_get_channel_buffer(chan); ///< The currently decoded message
//	mavlink_status_t* status = mavlink_get_channel_status(chan); ///< The current decode status
    int bufferIndex = 0;

    status->msg_received = 0;

    switch (status->parse_state)
    {
    case MAVLINK_PARSE_STATE_UNINIT:
    case MAVLINK_PARSE_STATE_IDLE:
        if (c == MAVLINK_STX)
        {
            status->parse_state = MAVLINK_PARSE_STATE_GOT_STX;
            rxmsg->len = 0;
            rxmsg->magic = c;
            mavlink_start_checksum(rxmsg);
        }
        break;

    case MAVLINK_PARSE_STATE_GOT_STX:
            if (status->msg_received
/* Support shorter buffers than the
   default maximum packet size */
#if (MAVLINK_MAX_PAYLOAD_LEN < 255)
                || c > MAVLINK_MAX_PAYLOAD_LEN
#endif
                )
        {
            status->buffer_overrun++;
            status->parse_error++;
            status->msg_received = 0;
            status->parse_state = MAVLINK_PARSE_STATE_IDLE;
        }
        else
        {
            // NOT counting STX, LENGTH, SEQ, SYSID, COMPID, MSGID, CRC1 and CRC2
            rxmsg->len = c;
            status->packet_idx = 0;
            mavlink_update_checksum(rxmsg, c);
            status->parse_state = MAVLINK_PARSE_STATE_GOT_LENGTH;
        }
        break;

    case MAVLINK_PARSE_STATE_GOT_LENGTH:
        rxmsg->seq = c;
        mavlink_update_checksum(rxmsg, c);
        status->parse_state = MAVLINK_PARSE_STATE_GOT_SEQ;
        break;

    case MAVLINK_PARSE_STATE_GOT_SEQ:
        rxmsg->sysid = c;
        mavlink_update_checksum(rxmsg, c);
        status->parse_state = MAVLINK_PARSE_STATE_GOT_SYSID;
        break;

    case MAVLINK_PARSE_STATE_GOT_SYSID:
        rxmsg->compid = c;
        mavlink_update_checksum(rxmsg, c);
        status->parse_state = MAVLINK_PARSE_STATE_GOT_COMPID;
        break;

    case MAVLINK_PARSE_STATE_GOT_COMPID:
#if MAVLINK_CHECK_MESSAGE_LENGTH
            if (rxmsg->len != ML_CONN_MAVLINK_MESSAGE_LENGTH(c))
        {
            status->parse_error++;
            status->parse_state = MAVLINK_PARSE_STATE_IDLE;
            break;
            if (c == MAVLINK_STX)
            {
                status->parse_state = MAVLINK_PARSE_STATE_GOT_STX;
                mavlink_start_checksum(rxmsg);
            }
            }
#endif
        rxmsg->msgid = c;
        mavlink_update_checksum(rxmsg, c);
        if (rxmsg->len == 0)
        {
            status->parse_state = MAVLINK_PARSE_STATE_GOT_PAYLOAD;
        }
        else
        {
            status->parse_state = MAVLINK_PARSE_STATE_GOT_MSGID;
        }
        break;

    case MAVLINK_PARSE_STATE_GOT_MSGID:
        _MAV_PAYLOAD_NON_CONST(rxmsg)[status->packet_idx++] = (char)c;
        mavlink_update_checksum(rxmsg, c);
        if (status->packet_idx == rxmsg->len)
        {
            status->parse_state = MAVLINK_PARSE_STATE_GOT_PAYLOAD;
        }
        break;

    case MAVLINK_PARSE_STATE_GOT_PAYLOAD:
#if MAVLINK_CRC_EXTRA
        mavlink_update_checksum(rxmsg, ML_CONN_MAVLINK_MESSAGE_CRC(rxmsg->msgid));
#endif
        if (c != (rxmsg->checksum & 0xFF)) {
            // Check first checksum byte
            status->parse_error++;
            status->msg_received = 0;
            status->parse_state = MAVLINK_PARSE_STATE_IDLE;
            if (c == MAVLINK_STX)
            {
                status->parse_state = MAVLINK_PARSE_STATE_GOT_STX;
                rxmsg->len = 0;
                mavlink_start_checksum(rxmsg);
            }
        }
        else
        {
            status->parse_state = MAVLINK_PARSE_STATE_GOT_CRC1;
            _MAV_PAYLOAD_NON_CONST(rxmsg)[status->packet_idx] = (char)c;
        }
        break;

    case MAVLINK_PARSE_STATE_GOT_CRC1:
        if (c != (rxmsg->checksum >> 8)) {
            // Check second checksum byte
            status->parse_error++;
            status->msg_received = 0;
            status->parse_state = MAVLINK_PARSE_STATE_IDLE;
            if (c == MAVLINK_STX)
            {
                status->parse_state = MAVLINK_PARSE_STATE_GOT_STX;
                rxmsg->len = 0;
                mavlink_start_checksum(rxmsg);
            }
        }
        else
        {
            // Successfully got message
            status->msg_received = 1;
            status->parse_state = MAVLINK_PARSE_STATE_IDLE;
            _MAV_PAYLOAD_NON_CONST(rxmsg)[status->packet_idx+1] = (char)c;
            memcpy(r_message, rxmsg, sizeof(mavlink_message_t));
        }
        break;
    }

    bufferIndex++;
    // If a message has been sucessfully decoded, check index
    if (status->msg_received == 1)
    {
        //while(status->current_seq != rxmsg->seq)
        //{
        //	status->packet_rx_drop_count++;
        //               status->current_seq++;
        //}
        status->current_rx_seq = rxmsg->seq;
        // Initial condition: If no packet has been received so far, drop count is undefined
        if (status->packet_rx_success_count == 0) status->packet_rx_drop_count = 0;
        // Count this packet as received
        status->packet_rx_success_count++;
    }

    r_mavlink_status->current_rx_seq = status->current_rx_seq+1;
    r_mavlink_status->packet_rx_success_count = status->packet_rx_success_count;
    r_mavlink_status->packet_rx_drop_count = status->parse_error;
    status->parse_error = 0;
    return status->msg_received;
}


#endif /* _MAVLINK_CONN_MAVLINK_PARSE_CHAR_H_ */

/******************************************************************** 
 *  pod_scan_escape.c
 *  Copyright (c) 2011-2012, James A. Banks
 *  All rights reserved.
 *  See file LICENSE for details.
 ********************************************************************/

#include "pod_char.h"
#include "pod_log.h"
#include "pod_scan.h"



    // pod_scan_escape
    //
    // At this point I have the leading backslash.  I am getting the escaped
    // character, or the first character of the escaped hex number.
    //
    // Returns:
    //      int     The error id of any problem that occurred (0 = no error)

int pod_scan_escape(pod_stream *stream, pod_char_t c)
{
    int w;

    w = POD_OKAY;
    switch (c) {
        case POD_CHAR_NEWLINE:
        case POD_CHAR_RETURN:
        case POD_CHAR_EOB:
            // There is no escape character, and the token ends.
            if (!pod_string_is_empty(stream->s_buffer)) {
                pod_stream_add_token(stream, POD_TOKEN_STRING);
            }
            if (c == POD_CHAR_EOB) {
                pod_stream_add_token(stream, POD_TOKEN_POD_SYNC);
            }
            stream->s_state = POD_STATE_START;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            stream->s_state &= POD_STATE_MASK;
            stream->s_state |= POD_STATE_ESCAPE_HEX;
            stream->escape = (c - '0') & 0xf;
            break;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            stream->s_state &= POD_STATE_MASK;
            stream->s_state |= POD_STATE_ESCAPE_HEX;
            stream->escape = (10 + (c - 'A')) & 0xf;
            break;
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            stream->s_state &= POD_STATE_MASK;
            stream->s_state |= POD_STATE_ESCAPE_HEX;
            stream->escape = (10 + (c - 'a')) & 0xf;
            break;
        case 't':
            w = pod_scan_append_to_buffer(stream->s_buffer, POD_CHAR_TAB);
            stream->s_state &= POD_STATE_MASK;
            break;
        case 'n':
            w = pod_scan_append_to_buffer(stream->s_buffer, POD_CHAR_NEWLINE);
            stream->s_state &= POD_STATE_MASK;
            break;
        case 'r':
            w = pod_scan_append_to_buffer(stream->s_buffer, POD_CHAR_RETURN);
            stream->s_state &= POD_STATE_MASK;
            break;
        case '\\':
            w = pod_scan_append_to_buffer(stream->s_buffer, POD_CHAR('\\'));
            stream->s_state &= POD_STATE_MASK;
            break;
        default:
            if (! POD_CHAR_IS_PRINTING(c)) {
                w = 1; // TODO
            } else {
                w = pod_scan_append_to_buffer(stream->s_buffer, c);
                stream->s_state &= POD_STATE_MASK;
            }
            break;
    }

    return w;
}

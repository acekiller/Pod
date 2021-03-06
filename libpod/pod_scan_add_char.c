/******************************************************************** 
 *  pod_scan_add_char.c
 *  Copyright (c) 2011-2012, James A. Banks
 *  All rights reserved.
 *  See file LICENSE for details.
 ********************************************************************/

#include "pod_scan.h"
#include "pod_log.h"



    // pod_scan_add_char
    //
    // Got sufficient input to constitute a character and have coverted it to a
    // pod_char_t.
    //
    // Returns:
    //      int     The error id of any problem that occurred (0 = no error)

int pod_scan_add_char(pod_stream *stream, pod_char_t c)
{
    int warning;

    warning = 0;
    if (stream->total_warnings >= 0) {
        if (stream->total_warnings >= stream->max_warnings) {
            // warn that the stream is ignoring any further input
            warning = POD_TOO_MANY_WARNINGS;
            pod_stream_log(stream, warning, __FILE__, __LINE__);
            return warning;
        }
    }
    ++stream->total_characters;
    ++stream->position;
    if (c == POD_CHAR_NEWLINE || c == POD_CHAR_RETURN) {
        stream->position = 1;
        ++stream->line;
    }
    switch (stream->s_state) {
        case POD_STATE_START:
            warning = pod_scan_start(stream, c);
            break;
        case POD_STATE_SIMPLE:
            warning = pod_scan_simple(stream, c);
            break;
        case POD_STATE_SIMPLE_ESCAPE:
            warning = pod_scan_escape(stream, c);
            break;
        case POD_STATE_SIMPLE_ESCAPE_HEX:
            warning = pod_scan_escape_hex(stream, c);
            break;
        case POD_STATE_QUOTED:
            warning = pod_scan_quoted(stream, c);
            break;
        case POD_STATE_QUOTED_ESCAPE:
            warning = pod_scan_escape(stream, c);
            break;
        case POD_STATE_QUOTED_ESCAPE_HEX:
            warning = pod_scan_escape_hex(stream, c);
            break;
        case pod_blurb_pre_size:
            warning = pod_scan_blurb_pre_size(stream, c);
            break;
        case pod_blurb_size:
            warning = pod_scan_blurb_size(stream, c);
            break;
        case pod_blurb_post_size:
            warning = pod_scan_blurb_post_size(stream, c);
            break;
        case pod_end_escape:
            if (c == POD_CHAR('\\')) {
                stream->s_state = POD_STATE_START;
            }
            break;
        case pod_end_line:
            if (c == POD_CHAR_NEWLINE || c == POD_CHAR_RETURN) {
                stream->s_state = POD_STATE_START;
            }
            break;
        case pod_end_pod:
            if (c == POD_CHAR_EOB) {
                stream->s_state = POD_STATE_START;
            }
            break;
        default:
            warning = POD_INVALID_STREAM_STATE;
            pod_stream_log(stream, warning, __FILE__, __LINE__);
            stream->s_state = POD_STATE_START;
            break;
    }
    if (warning != 0) {
        ++stream->total_warnings;
    }

    return warning;
}


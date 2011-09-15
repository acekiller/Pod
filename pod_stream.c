#include "pod_stream.h"
#include "scanner.h"



    // pod_stream_add_char
    //
    // Got sufficient input to constitute a character and have coverted it to a
    // pod_char_t.
    //
    // Returns:
    //      int     The error id of any problem that occurred (0 = no error)

int pod_stream_add_char(pod_stream *stream, pod_char_t c, pod_object **object)
{
    int warning;

    warning = 0;
    if (stream->total_warnings >= 0) {
        if (stream->total_warnings >= stream_max_warnings) {
            // warn that the stream is ignoring any further input
            warning = POD_TOO_MANY_WARNINGS;
            pod_stream_log(stream, warning, __FILE__, __LINE__);
            return warning;
        }
    }
    ++stream->total_characters;
    ++stream->position;
    if (c == '\n' || c == '\r') {
        stream->position = 1;
        ++stream->line;
    }
    stream->c = c;
    switch (stream->state) {
        case stream_start:
            warning = scan_start(stream, c, object);
            break;
        case stream_simple:
            warning = scan_simple(stream, c, object);
            break;
        case stream_simple_escape:
            warning = scan_escape(stream, c, object);
            break;
        case stream_simple_escape_hex:
            warning = scan_escape_hex(stream, c, object);
            break;
        case stream_quoted:
            warning = scan_quoted(stream, c, object);
            break;
        case stream_quoted_escape:
            warning = scan_escape(stream, c, object);
            break;
        case stream_quoted_escape_hex:
            warning = scan_escape_hex(stream, c, object);
            break;
        case stream_blurb:
        case stream_blurb_pre_size:
            warning = scan_blurb_pre_size(stream, c, object);
            break;
        case stream_blurb_size:
            warning = scan_blurb_size(stream, c, object);
            break;
        case stream_blurb_post_size:
            warning = scan_blurb_post_size(stream, c, object);
            break;
        case stream_end_escape:
            if (c == '\\') {
                stream->state = stream_start;
            }
            break;
        case stream_end_line:
            if (c == '\n' || c == '\r') {
                stream->state = stream_start;
            }
            break;
        case stream_end_pod:
            if (c == '') {
                stream->state = stream_start;
            }
            break;
        default:
            warning = POD_INVALID_STREAM_STATE;
            pod_stream_log(stream, warning, __FILE__, __LINE__);
            stream->state = stream_start;
            break;
    }
    if (warning != 0) {
        ++stream->total_warnings;
    }

    return warning;
}



int pod_stream_add_token(pod_stream *stream, pod_stream_token token)
{
    pod_string *previous;
    pod_string *current;
    pod_map *map;
    pod_object *top;
    pod_object *new;
    pod_object *nothing;

    top = pod_list_peek(stream->stack);
    switch (token) {
        case string:
            current = pod_string_dup_text(stream->buffer);
            stream->buffer->used = 0;
            stream->have_concat = false;
            if (top->type == POD_STRING_TYPE) {
                previous = (pod_string *) top;
                if (previous->flags & POD_IS_KEY == 1) {
                    pop the string
                    pod_mapping_create_with(previous, (pod_object *) current);
                    make new map
                    push on stack
                    pod_stream_merge_down(stream);
                } else { 
                    pod_stream_merge_down(stream);
                    pod_list_push(stream->stack, current);
                }
            } else {
                pod_list_push(stream->stack, current);
            }
            break;
        case begin_map:
            if (top->type == POD_STRING_TYPE) {
                pod_stream_merge_down(stream);
            }
            map = pod_create_map();
            pod_list_push(stream->stack, (pod_object *) map);
            break;
        case equals:
        //mapping = pod_mapping_create();
        //mapping->key = top;
        //pod_list_push(stream->stack, mapping);
            if (top->type == POD_STRING_TYPE) {
                pop the string
                make a new mapping with the string as the key
                push it on the stack
            } else {
                // syntax error.  Need key
            }
            break;
        case end_map:
            if (top->type == POD_STRING_TYPE) {
                merge it down, which will generate an error
                // Error, unkeyed object
            } else if (top->type == POD_MAPPING_TYPE) {
                merge it down
                // Error, truncated mapping
            }
            top = pod_list_peek(stream->stack);
            if (top->type == POD_MAP_TYPE) {
                merge it down
            } else {
                // Error, encountered unmatched ">"
            }
            break;

        case begin_blurb:
        case end_blurb:
            break;

        case begin_list:
            if (top->type == POD_STRING_TYPE) {
                pod_stream_merge_down(stream);
            }
            list = pod_list_create()
            pod_list_push(stream->stack, (pod_object *) list);
            break;
        case end_list:
            if (top->type == POD_STRING_TYPE) {
                merge it down
            } else if (top->type == POD_MAPPING_TYPE) {
                merge it down
                // Error, truncated mapping
            }
            top = pod_list_peek(stream->stack);
            if (top->type == POD_LIST_TYPE) {
                merge it down
            } else {
                // Error, encountered unmatched "}"
            }
            break;
        case pod_sync:
            while there is something on the stack
                merge it down
            break;
        default:
            break;
}



void pod_stream_log(pod_stream *stream, int message, char *file_name, int line)
{
    // Example:
    //
    // Pod message 127: Got illegal character ')'
    //   stream: std_out
    //   at character xxxx (line xxx, column xx)
    //   message from pod_stream.c, line xxx
}



    // pod_stream_merge_down
    //
    // This routine takes whatever is on top of the stack and adds it into to
    // the next element down.  This only works if this second element is a
    // structure (that is, a list or a map).  Furthermore, if it's a map, then
    // only a mapping can be added.  If this routine is called with the top
    // element as a string with the POD_IS_KEY flag set, the routine will clear
    // the POD_IS_KEY flag.  So this routine shouldn't be called by the "="
    // token.
    //
    // Returns:
    //      int     The error id of any problem that occurred (0 = no error)

int pod_stream_merge_down(pod_stream *stream)
{
    int is_mapping;
    pod_mapping *mapping;
    pod_object *top;
    pod_object *popped;
    pod_object *second;
    pod_string *string;
    int warning;

    warning = 0;

    top = pod_list_peek(stream->stack);

    if (top == NULL) {
        // The stack is empty, nothing to do
        return 0;
    } else if (top->type == POD_MAPPING_TYPE) {
        // Error: trailing "="
        // ie, foo = . -> foo= . -> foo(error)
        mapping = (pod_mapping *) pod_list_pop(stream->stack);
        pod_list_push(mapping->key);
        mapping->key = NULL;
        mapping->o.destroy(mapping);
    }

    popped = pod_list_pop(stream->stack);
    top = pod_list_peek(stream->stack);

    if (top == NULL) {
        // The stack is now empty, so popped is the whole pod.
        stream->process_pod(popped);
    } else if (top->type == POD_LIST_TYPE) {
        // The top of the stack is a list, so add popped.
        pod_list_append((pod_list *) top, popped);
    } else if (top->type == POD_MAPPING_TYPE) {
        if (popped->type != POD_MAPPING_TYPE) {
            ((pod_mapping *) top)->value = popped;
            warning = pod_stream_merge_down(stream);
        } else {
            // Error: a mapping cannot be the value of another mapping
            // I don't think this can happen
            popped->destroy(popped);
        }
    } else if (top->type == POD_MAP_TYPE) {
        if (popped->type == POD_MAPPING_TYPE) {
            pod_map_define_mapping((pod_map *) top, popped);
        } else {
            // Error: tried to insert unnamed object into map
            // ie, < foo >
            popped->destroy(popped);
        }
    } else {
        // Error: tried to insert element into string
        // I don't think this can happen
        popped->destroy(popped);
    }

    return warning;
}

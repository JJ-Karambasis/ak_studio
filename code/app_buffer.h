#ifndef APP_BUFFER_H
#define APP_BUFFER_H

typedef struct {
	usize CodepointAt;
	usize CodepointCount;
} line;

typedef struct {
	memory_reserve Reserve;
	usize 		   Count;
	line* 		   Ptr;
} line_array;

typedef struct {
	u32 Value;
} codepoint;

typedef struct {
	memory_reserve Reserve;
	usize 		   Count;
	codepoint*     Ptr;
} codepoint_array;

typedef struct {
	codepoint_array Codepoints;
	line_array  	Lines;
} app_buffer;

#endif
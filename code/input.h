#ifndef INPUT_H
#define INPUT_H

typedef struct {
	f64 RepeatingTime;
	b32 WasDown;
	b32 IsDown;
	b32 IsRepeating;
} button;

typedef enum {
	KEYBOARD_KEY_ALT       = 256,
	KEYBOARD_KEY_SHIFT     = 257,
	KEYBOARD_KEY_CTRL      = 258,
	KEYBOARD_KEY_F1        = 259,
	KEYBOARD_KEY_F2        = 260,
	KEYBOARD_KEY_F3        = 261,
	KEYBOARD_KEY_F4        = 262,
	KEYBOARD_KEY_F5        = 263,
	KEYBOARD_KEY_F6        = 264,
	KEYBOARD_KEY_F7        = 265,
	KEYBOARD_KEY_F8        = 266,
	KEYBOARD_KEY_F9        = 267,
	KEYBOARD_KEY_F10       = 268,
	KEYBOARD_KEY_F11       = 269,
	KEYBOARD_KEY_F12       = 270,
	KEYBOARD_KEY_LEFT      = 271,
	KEYBOARD_KEY_RIGHT     = 272,
	KEYBOARD_KEY_DOWN      = 273,
	KEYBOARD_KEY_UP        = 274,
	KEYBOARD_KEY_DELETE    = 275,
	KEYBOARD_KEY_BACKSPACE = 276,
	KEYBOARD_KEY_COUNT     = 277
} keyboard_key;

typedef enum {
	MOUSE_KEY_LEFT,
	MOUSE_KEY_MIDDLE,
	MOUSE_KEY_RIGHT,
	MOUSE_KEY_COUNT
} mouse_key;

typedef struct {
	memory_reserve Reserve;
	usize Used;
} char_stream;

typedef struct {
	char_stream CharStream;
	button      Keyboard[KEYBOARD_KEY_COUNT];
	button      Mouse[MOUSE_KEY_COUNT];
} input;

#endif
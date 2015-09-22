#include "input.h"
#include <SDL2/SDL.h>

typedef struct
{
	Uint8 stack[CT_MAX_INPUT_STACK_SIZE];
	Uint8 size;
} InputStack;

static InputStack pressed_stack, released_stack, holded_stack;

static int _ct_is_quitting = 0;

static int has_key(CT_Key key, InputStack* stack)
{
	unsigned i;
	for (i=0; i<CT_MAX_INPUT_STACK_SIZE; i++)
	{
		if (stack->stack[i] == key) return 1;
	}
	return 0;
}

static void key_push(CT_Key key, InputStack* stack)
{
	if (key && !has_key(key, stack))
	{
		if (stack->size == CT_MAX_INPUT_STACK_SIZE) stack->size = 0;
		stack->stack[stack->size++] = key;
	}
}

static void key_pop(CT_Key key, InputStack* stack)
{
	unsigned i;
	for (i=0; i<CT_MAX_INPUT_STACK_SIZE; i++)
	{
		if (stack->stack[i] == key)
		{
			stack->stack[i] = 0;
		}
	}
}

static void key_down_callback(Uint8 key)
{
	if (!has_key(key, &holded_stack)) key_push(key, &pressed_stack);
}

static void key_up_callback(Uint8 key)
{
	key_pop(key, &holded_stack);
	key_push(key, &released_stack);
}

static void reset_stacks()
{
	unsigned i;
	for (i=0; i<CT_MAX_INPUT_STACK_SIZE; i++)
	{
		CT_Key key = pressed_stack.stack[i];
		/* Up-event for mouse button-x1 and button-x2 don't seem to
		   work all that well in SDL. To prevent them getting stuck
		   we won't add them to the holded buffer. */
		if (key != CT_BUTTON_X1 && key != CT_BUTTON_X2)
		{
			key_push(key, &holded_stack);
		}
		pressed_stack.stack [i] = 0;
		released_stack.stack[i] = 0;
	}
}

int ct_is_quitting()
{
	return _ct_is_quitting;
}

int ct_is_key_pressed(CT_Key key)
{
	return has_key(key, &pressed_stack);
}

int ct_is_key_released(CT_Key key)
{
	return has_key(key, &released_stack);
}

int ct_is_key_holded(CT_Key key)
{
	return has_key(key, &holded_stack);
}

void ct_mouse_position(float* ret)
{
	int x, y;
	SDL_GetMouseState(&x, &y);
	ret[0] = x;
	ret[1] = y;
}

unsigned ct_holded_keys(CT_Key* ret)
{
	unsigned i, count = 0;
	for (i=0; i<CT_MAX_INPUT_STACK_SIZE; i++)
	{
		CT_Key key = holded_stack.stack[i];
		if (key)
		{
			ret[count] = key;
			count++;
		} else ret[i] = 0;
	}
	return count;
}

void ct_poll_input()
{
	reset_stacks();

	SDL_Event event;
	while(SDL_PollEvent(&event) == 1)
	{
		switch(event.type) {
		case SDL_MOUSEBUTTONDOWN:
			key_down_callback(event.button.button);
			break;
		case SDL_MOUSEBUTTONUP:
			key_up_callback(event.button.button);
			break;
		case SDL_KEYDOWN:
			key_down_callback(event.key.keysym.sym);
			break;
		case SDL_KEYUP:
			key_up_callback(event.key.keysym.sym);
			break;
		case SDL_QUIT:
			_ct_is_quitting = 1;
			break;
		}
	}
}

static const char* key_names[] = {
	"no_key",
	"button_left",
	"button_middle",
	"button_right",
	"button_wheelup",
	"button_wheeldown",
	"button_x1",
	"button_x2",
	"backspace",
	"tab",
	"unknown",
	"unknown",
	"clear",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"pause",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"escape",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"space",
	"exclaim",
	"quotedbl",
	"hash",
	"dollar",
	"unknown",
	"ampersand",
	"quote",
	"leftparen",
	"rightparen",
	"asterisk",
	"plus",
	"comma",
	"minus",
	"period",
	"slash",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"colon",
	"semicolon",
	"less",
	"equals",
	"greater",
	"question",
	"at",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"leftbracket",
	"backslash",
	"rightbracket",
	"caret",
	"underscore",
	"backquote",
	"a",
	"b",
	"c",
	"d",
	"e",
	"f",
	"g",
	"h",
	"i",
	"j",
	"k",
	"l",
	"m",
	"n",
	"o",
	"p",
	"q",
	"r",
	"s",
	"t",
	"u",
	"v",
	"w",
	"x",
	"y",
	"z",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"delete",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"unknown",
	"kp0",
	"kp1",
	"kp2",
	"kp3",
	"kp4",
	"kp5",
	"kp6",
	"kp7",
	"kp8",
	"kp9",
	"keypad_period",
	"keypad_divide",
	"keypad_multiply",
	"keypad_minus",
	"keypad_plus",
	"keypad_enter",
	"keypad_equals",
	"up",
	"down",
	"right",
	"left",
	"insert",
	"home",
	"end",
	"pageup",
	"pagedown",
	"f1",
	"f2",
	"f3",
	"f4",
	"f5",
	"f6",
	"f7",
	"f8",
	"f9",
	"f10",
	"f11",
	"f12",
	"f13",
	"f14",
	"f15",
	"unknown",
	"unknown",
	"unknown",
	"numlock",
	"capslock",
	"scrollock",
	"right shift",
	"left shift",
	"right ctrl",
	"left ctrl",
	"right alt",
	"left alt",
	"right meta",
	"left meta",
	"left super",
	"right super",
	"mode",
	"compose",
	"help",
	"print",
	"sysreq",
	"break",
	"menu",
	"power",
	"euro",
	"undo"
};

const char* ct_key_name(CT_Key key)
{
	return key_names[key];
}

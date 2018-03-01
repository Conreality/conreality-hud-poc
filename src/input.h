/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INPUT_H
#define INPUT_H

/*These are only applicable to a specific kind of joystick*/
#define JOYSTICK_SELECT 0
#define JOYSTICK_LEFT_THUMB 1
#define JOYSTICK_RIGHT_THUMB 2
#define JOYSTICK_START 3
#define JOYSTICK_DPAD_UP 4
#define JOYSTICK_DPAD_RIGHT 5
#define JOYSTICK_DPAD_DOWN 6
#define JOYSTICK_DPAD_LEFT 7
#define JOYSTICK_LEFT_TRIGGER 8
#define JOYSTICK_RIGHT_TRIGGER 9
#define JOYSTICK_LEFT_BUMPER 10
#define JOYSTICK_RIGHT_BUMPER 11
#define JOYSTICK_TRIANGLE 12
#define JOYSTICK_CIRCLE 13
#define JOYSTICK_CROSS 14
#define JOYSTICK_SQUARE 15
#define JOYSTICK_HOME 16

void handleKey(GLFWwindow* window, int key, int code, int action, int modifiers);
void handleMouseButton(GLFWwindow* window, int button, int action, int mods);
void handleJoystick(int joystick);
void handleEvents();

#endif //INPUT_H

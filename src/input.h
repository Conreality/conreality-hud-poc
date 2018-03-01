/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INPUT_H
#define INPUT_H

void handleKey(GLFWwindow* window, int key, int code, int action, int modifiers);
void handleMouseButton(GLFWwindow* window, int button, int action, int mods);
void handleEvents();

#endif //INPUT_H

/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "globals.h"
#include "input.h"

extern Globals global;

void handleKey(GLFWwindow* window, int key, int code, int action, int modifiers) {
  global.kb_control_queue.try_emplace(kb_control_input{key, code, action, modifiers});
}

void handleMouseButton(GLFWwindow* window, int button, int action, int mods) {
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  global.ms_control_queue.try_emplace(ms_control_input{button, action, mods, xpos, ypos});
}

void handleJoystick(int joystick) {
/*Buttons only report a state of pressed/unpressed*/
  int buttons_count;
  const unsigned char* buttons = glfwGetJoystickButtons(joystick, &buttons_count);
/*
  for (int i = 0; i < buttons_count; i++) {
    std::printf("%d\n", buttons[i]);
  }
  std::cout << std::endl << std::endl;
*/

/*Axes can be used to check how hard a button is being pressed, and also which way the thumbsticks are pointing*/
  int axes_count;
  const float* axes = glfwGetJoystickAxes(joystick, &axes_count);
/*
  for (int i = 0; i < axes_count; i++) {
    std::printf("%f\n", axes[i]);
  }
  std::cout << std::endl << std::endl;
*/

  global.js_control_queue.try_emplace(js_control_input{buttons, axes});
}

void handleEvents() {

  kb_control_input kb_event;
  global.kb_control_queue.try_pop(kb_event);

  if (kb_event.action == GLFW_PRESS) {
//    std::printf("%d \n", kb_event.key);
    switch (kb_event.key) {
      case GLFW_KEY_ESCAPE:
        global.flags.is_running = false;
        break;
      case GLFW_KEY_E:
        if (kb_event.modifiers == GLFW_MOD_SHIFT) { global.flags.edge_filter_ext = !(global.flags.edge_filter_ext); break; }
        global.flags.edge_filter = !(global.flags.edge_filter);
        break;
      case GLFW_KEY_F:
        global.flags.flip_image = !(global.flags.flip_image);
        break;
      case GLFW_KEY_I:
        global.flags.show_items = !(global.flags.show_items);
        break;
      default:
        break;
    }
  }

  ms_control_input ms_event;
  global.ms_control_queue.try_pop(ms_event);

  if (ms_event.action == GLFW_PRESS) {
//    std::cout << ms_event.xpos << ", " << ms_event.ypos << std::endl;
  }

  js_control_input js_event;
  global.js_control_queue.try_pop(js_event);
/*
  if (js_event.buttons[JOYSTICK_CROSS]) { std::printf("Cross\n"); }
  if (js_event.buttons[JOYSTICK_CIRCLE]) { std::printf("Circle\n"); }
  if (js_event.buttons[JOYSTICK_TRIANGLE]) { std::printf("Triangle\n"); }
  if (js_event.buttons[JOYSTICK_SQUARE]) { std::printf("Square\n"); }
*/
}

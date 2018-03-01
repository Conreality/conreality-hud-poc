/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <thread>
#include <future>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <tbb/concurrent_queue.h>

#ifndef DISABLE_OSVR
#include <osvr/ClientKit/ClientKit.h>
#include <osvr/ClientKit/Display.h>
#endif

struct kb_control_input {
  int key, code, action, modifiers;
};

struct ms_control_input {
  int button, action, mods;
  double xpos, ypos;
};

struct js_control_input {
   const unsigned char* buttons;
   const float* axes;
};

struct Globals {

  struct {
    bool is_running = true;
    bool show_items = false;
    bool flip_image = false;
    bool edge_filter = false;
    bool edge_filter_ext = false;
    bool joystick_connected = true;
    bool fullscreen = true;
    bool save_output_videofile = false;
  } flags;

  tbb::concurrent_bounded_queue<kb_control_input> kb_control_queue;
  tbb::concurrent_bounded_queue<ms_control_input> ms_control_queue;
  tbb::concurrent_bounded_queue<js_control_input> js_control_queue;
};

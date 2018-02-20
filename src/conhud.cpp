/* This is free and unencumbered software released into the public domain. */

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

#ifdef ENABLE_OSVR
#include <osvr/ClientKit/ClientKit.h>
#include <osvr/ClientKit/Display.h>
#endif

#include "yolo_v2_class.hpp"

struct image_data {
  cv::Mat frame;
};

struct kb_control_input {
  int key, code, action, modifiers;
};

struct ms_control_input {
  int button, action, mods;
  double xpos, ypos;
};

struct {

  struct {
    bool is_running = true;
    bool show_items = false;
    bool flip_image = false;
    bool fullscreen = true;
    bool save_output_videofile = false;
  } flags;

  tbb::concurrent_bounded_queue<kb_control_input> kb_control_queue;
  tbb::concurrent_bounded_queue<ms_control_input> ms_control_queue;
} global;

void handleKey(GLFWwindow* window, int key, int code, int action, int modifiers);
void handleMouseButton(GLFWwindow* window, int button, int action, int mods);
static void handleEvents();
#ifdef ENABLE_OSVR
void render(osvr::clientkit::DisplayConfig &display, cv::Mat frame, GLuint texture, int window_w, int window_h);
#endif
void drawToGLFW(cv::Mat img, GLuint texture, int window_w, int window_h);
void drawSquare(float x, float y, int w, int h);
void drawCircle(float cx, float cy, float r);
void drawBoxes(cv::Mat mat_img, std::vector<bbox_t> result_vec, std::vector<std::string> object_names);
void showConsoleResult(std::vector<bbox_t> const result_vec, std::vector<std::string> const object_names);
cv::Scalar objectIdToColor(int obj_id);
std::vector<std::string> objectNamesFromFile(std::string const filename);
void printHelp();

/********************
*                   *
*   MAIN FUNCTION   *
*                   *
********************/

int main(int argc, char* argv[]) {

  std::string filename = "0";
  std::string names_file = "darknet/data/voc.names";
  std::string cfg_file = "darknet/cfg/tiny-yolo-voc.cfg";
  std::string weights_file = "darknet/tiny-yolo-voc.weights";
//std::string cfg_file = "darknet/cfg/yolo-voc.cfg";
//std::string weights_file = "darknet/yolo-voc.weights";

/*used for limiting clock speed*/
  const int FPS = 30;
  const int FRAME_DELAY = 1000 / FPS;
  auto next_frame = std::chrono::steady_clock::now();

/*set default screen size*/
  int window_h = 1080;
  int window_w = 1920;

  std::string out_videofile = "result.avi";

  int c;
  while ((c = getopt(argc, argv, "hrs:w")) != -1) {
    switch (c) {
      case 'h': //help
        printHelp();
        return EXIT_SUCCESS;
      case 'r': //record
        global.flags.save_output_videofile = true;
        break;
      case 's': //source
        filename = optarg;
        break;
      case 'w': //windowed
        global.flags.fullscreen = 0;
        window_h = 504; //16:9
        window_w = 896;
        break;
      case '?':
        if (optopt == 's') {
          std::printf("Option -%c requires an argument.\n", optopt);
        } else if (isprint(optopt)) {
          std::printf("Unknown option '-%c'\n", optopt);
        }
        return EXIT_FAILURE;
      default:
        break;
    }
  }

  Detector detector(cfg_file, weights_file);

  auto object_names = objectNamesFromFile(names_file);

  GLuint texture;

  if (!glfwInit() ) {
    std::fprintf(stderr, "Failed to initialize GLFW\n");
    return EXIT_FAILURE;
  }

  GLFWwindow* window;
  window = glfwCreateWindow(window_w, window_h, "Window", NULL, NULL);
  if (window == NULL) {
    std::fprintf(stderr, "Failed to open GLFW window.\n");
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    std::fprintf(stderr, "Failed to initialize GLEW\n");
    return EXIT_FAILURE;
  }

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetKeyCallback(window, handleKey);
  glfwSetMouseButtonCallback(window, handleMouseButton);

  glfwSwapInterval(1);

  glViewport(0, 0, window_w, window_h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, window_w, window_h, 0.0, 0.0, 100.0);
  glMatrixMode(GL_MODELVIEW);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearDepth(0.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

#ifdef ENABLE_OSVR
  osvr::clientkit::ClientContext ctx("com.osvr.conreality.hud");
  osvr::clientkit::DisplayConfig display(ctx);

  if (!display.valid()) {
    std::printf("Failed to get display config!\n");
    return EXIT_FAILURE;
  }

  while (!display.checkStartup()) {
    ctx.update();
  }
#endif

  std::string file_ext = filename.substr(filename.find_last_of(".") + 1);

  cv::VideoCapture capture;
  if (file_ext == "avi" || file_ext == "mp4" || file_ext == "mpjg" || file_ext == "mov") {
     capture.open(filename);
  } else if (isdigit(filename[0])) {
    capture.open(stoi(filename));
  }
  if (!capture.isOpened()) { std::printf("Failed to open camera!\n"); return EXIT_FAILURE; }

//capture.set(CV_CAP_PROP_FPS, 60);
//capture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
//capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

  cv::Mat capt_frame;

  capture >> capt_frame;
  if (capt_frame.empty()) { std::printf("Failed to capture a frame!\n"); return EXIT_FAILURE; }
  cv::Size const frame_size = capt_frame.size();

  image_data capt_image, pros_image;

  tbb::concurrent_bounded_queue<image_data> image_queue;
  image_queue.set_capacity(10);

  global.kb_control_queue.set_capacity(5);
  global.ms_control_queue.set_capacity(5);

  std::vector<bbox_t> result_vec;

  cv::VideoWriter output_video;
  if (global.flags.save_output_videofile) {
    output_video.open(out_videofile, CV_FOURCC('D','I','V','X'), std::max(35, 30), frame_size, true);
  }

  std::thread t_capture;
  t_capture = std::thread([&]() {
    while(global.flags.is_running) {
      if (image_queue.size() < 10) {
        capture >> capt_frame;
        capt_image.frame = capt_frame.clone();
        image_queue.try_push(capt_image);
      }
    }
  });

/**************
*  main loop  *
**************/

  while (global.flags.is_running) {

    next_frame += std::chrono::milliseconds(FRAME_DELAY);

    handleEvents();

    if (!image_queue.empty()) {
      image_queue.try_pop(pros_image);

      if (pros_image.frame.empty()) { std::printf("Video feed has ended\n"); global.flags.is_running = false; }

/*detect objects and draw a box around them*/
      result_vec = detector.detect(pros_image.frame);
      drawBoxes(pros_image.frame, result_vec, object_names);
//    showConsoleResult(result_vec, object_names);    //uncomment this if you want console feedback

      if (global.flags.flip_image) { cv::flip(pros_image.frame, pros_image.frame, 0); }

/*update and render video feed*/
#ifdef ENABLE_OSVR
      ctx.update();
      if (global.flags.fullscreen) { render(display, pros_image.frame, texture, window_w, window_h); }
      else {
        glViewport(0, 0, window_w, window_h);
        drawToGLFW(pros_image.frame, texture, window_w, window_h);
      }
#else
      glViewport(0, 0, window_w, window_h);
      drawToGLFW(pros_image.frame, texture, window_w, window_h);
#endif
      glfwSwapBuffers(window);
      glfwPollEvents();

      if (output_video.isOpened() && global.flags.save_output_videofile) {
        output_video << pros_image.frame;
      }
    }

/*slow down to 30FPS if running faster*/
    std::this_thread::sleep_until(next_frame);

  } //main loop

  if (t_capture.joinable()) { t_capture.join(); }

  glfwTerminate();

  cv::destroyAllWindows();

  std::printf("Program exited\n");

  return EXIT_SUCCESS;
}

/********************
*                   *
*     FUNCTIONS     *
*                   *
********************/

/*keyboard input*/
void handleKey(GLFWwindow* window, int key, int code, int action, int modifiers) {
  global.kb_control_queue.try_emplace(kb_control_input{key, code, action, modifiers});
}

void handleMouseButton(GLFWwindow* window, int button, int action, int mods) {
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  global.ms_control_queue.try_emplace(ms_control_input{button, action, mods, xpos, ypos});
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
}

#ifdef ENABLE_OSVR
void render(osvr::clientkit::DisplayConfig &display, cv::Mat frame, GLuint texture, int window_w, int window_h) {
  display.forEachEye([&](osvr::clientkit::Eye eye) {
    eye.forEachSurface([&](osvr::clientkit::Surface surface) {
      uint8_t eye_number = eye.getEyeID();
      auto viewport = surface.getRelativeViewport();
      glViewport(static_cast<GLint>(viewport.left),
                 static_cast<GLint>(viewport.bottom),
                 static_cast<GLsizei>(viewport.width),
                 static_cast<GLsizei>(viewport.height));
      drawToGLFW(frame, texture, window_w, window_h);
      if (global.flags.show_items) {
        if (eye_number == 0) { drawSquare(window_w/2.0f, window_h/2.0f, 200, 150); }
        if (eye_number == 1) { drawCircle(window_w/2.0f, window_h/2.0f, 100.0f); }
      }
    });
  });
}
#endif

void drawToGLFW(cv::Mat img, GLuint texture, int window_w, int window_h) {

  glLoadIdentity();
  glGenTextures(1, &texture);
  glMatrixMode(GL_MODELVIEW);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.size().width, img.size().height, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);

  glEnable(GL_TEXTURE_2D);

  glColor4f(1.0f, 1.0f, 1.0f, 0.0f);

  glBegin(GL_QUADS);
    glTexCoord2i(0, 0); glVertex2i(0, 0);
    glTexCoord2i(0, 1); glVertex2i(0, window_h);
    glTexCoord2i(1, 1); glVertex2i(window_w, window_h);
    glTexCoord2i(1, 0); glVertex2i(window_w, 0);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glDeleteTextures(1, &texture);
}

void drawSquare(float x, float y, int w, int h) {
  x -= h/2; //to center the square
  y -= w/2;
  glBegin(GL_POLYGON);
  glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
  glVertex3f(x,y,0);
  glVertex3f(x+w,y,0);
  glVertex3f(x+w,y+h,0);
  glVertex3f(x,y+h,0);
  glEnd();
}

void drawCircle(float x, float y, float radius) {

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(x, y, 0.0f);
  static const int circle_points = 100;
  static const float angle = 2.0f * 3.1416f / circle_points;

  glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
  glBegin(GL_POLYGON);
  double angle1 = 0.0;
  glVertex2d(radius * cos(0.0) , radius * sin(0.0));
  for (int i=0; i < circle_points; i++) {
    glVertex2d(radius * cos(angle1), radius *sin(angle1));
    angle1 += angle;
  }
  glEnd();
  glPopMatrix();
}

void drawBoxes(cv::Mat mat_img, std::vector<bbox_t> result_vec, std::vector<std::string> object_names) {
  int const colors[6][3] = { {1,0,1},{0,0,1},{0,1,1},{0,1,0},{1,1,0},{1,0,0} };
  for (auto &i : result_vec) {
    cv::Scalar color = objectIdToColor(i.obj_id);
    cv::rectangle(mat_img, cv::Rect(i.x, i.y, i.w, i.h), color, 5);
    if (object_names.size() > i.obj_id) {
      std::string obj_name = object_names[i.obj_id];
      if (i.track_id > 0) { obj_name += " - " + std::to_string(i.track_id); }
      cv::Size const text_size = getTextSize(obj_name, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, 2, 0);
      int const max_width = (text_size.width > i.w + 2) ? text_size.width : (i.w + 2);
      cv::rectangle(mat_img, cv::Point2f(std::max((int)i.x - 3, 0), std::max((int)i.y - 30, 0)), cv::Point2f(std::min((int)i.x + max_width, mat_img.cols - 1), std::min((int)i.y, mat_img.rows - 1)), color, CV_FILLED, 8, 0);
      putText(mat_img, obj_name, cv::Point2f(i.x, i.y -10), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, cv::Scalar(0,0,0), 2);
    }
  }
}

cv::Scalar objectIdToColor(int obj_id) {
  int const colors[6][3] = { {1,0,1},{0,0,1},{0,1,1},{0,1,0},{1,1,0},{1,0,0} };
  int const offset = obj_id * 123457 % 6;
  int const color_scale = 150 + (obj_id * 123457) % 100;
  cv::Scalar color(colors[offset][0],colors[offset][1],colors[offset][2]);
  color *= color_scale;
  return color;
}

void showConsoleResult(std::vector<bbox_t> const result_vec, std::vector<std::string> const object_names) {
  for (auto &i : result_vec) {
    if (object_names.size() > i.obj_id) { std::cout << object_names[i.obj_id] << " - "; }
    std::cout << "obj_id = " << i.obj_id << ",  x = " << i.x << ", y = " << i.y
      << ", w = " << i.w << ", h = " << i.h
      << std::setprecision(3) << ", prob = " << i.prob << std::endl;
  }
}

std::vector<std::string> objectNamesFromFile(std::string const filename) {
  std::ifstream file(filename);
  std::vector<std::string> file_lines;
  if (!file.is_open()) { return file_lines; }
  for (std::string line; getline(file, line);) { file_lines.push_back(line); }
  std::printf("Object names loaded\n");
  return file_lines;
}

void printHelp() {
  std::printf("help\n");
}

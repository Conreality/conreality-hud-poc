#include <SDL.h>
#include <SDL_opengl.h>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <queue>

#include <osvr/ClientKit/ClientKit.h>
#include <osvr/ClientKit/Display.h>

#include "SDL2Helpers.h"

#include "yolo_v2_class.hpp"

GLuint texture;
void render(osvr::clientkit::DisplayConfig &display, cv::Mat frame);
void drawToHMD(cv::Mat img);
void drawBoxes(cv::Mat mat_img, std::vector<bbox_t> result_vec, std::vector<std::string> object_names);
void showConsoleResult(std::vector<bbox_t> const result_vec, std::vector<std::string> const object_names);
cv::Scalar objectIdToColor(int obj_id);
std::vector<std::string> objectNamesFromFile(std::string const filename);
static void handleEvents(bool* running_status);
void printHelp();

struct image_data {
  cv::Mat frame;
};

/********************
*                   *
*   MAIN FUNCTION   *
*                   *
********************/

int main(int argc, char* argv[]) {

  std::string filename = "1";
  std::string names_file = "darknet/data/voc.names";
  std::string cfg_file = "darknet/cfg/tiny-yolo-voc.cfg";
  std::string weights_file = "darknet/tiny-yolo-voc.weights";
//std::string cfg_file = "darknet/cfg/yolo-voc.cfg";
//std::string weights_file = "darknet/yolo-voc.weights";

/*for brevity*/
  const auto CENTERED = SDL_WINDOWPOS_CENTERED;

/*used for limiting clock speed*/
  const int FPS = 30;
  const int FRAME_DELAY = 1000 / FPS;
  int frame_start;
  int frame_time;

/*screen is set to fullscreen by default*/
  bool fullscreen = 1;
  int window_h = 1080;
  int window_w = 1920;

  std::string out_videofile = "result.avi";
  bool save_output_videofile = false;

  int c;
  while ((c = getopt(argc, argv, "hrs:w")) != -1) {
    switch (c) {
      case 'h': //help
        printHelp();
        return EXIT_SUCCESS;
      case 'r': //record
        save_output_videofile = true;
        break;
      case 's': //source
        filename = optarg;
        break;
      case 'w': //windowed
        fullscreen = 0;
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

  SDL_Init(SDL_INIT_VIDEO);

  SDL_Log("Current display mode is %dx%dpx", window_w, window_h);

  osvr::SDL2::Lib lib;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

  auto window = osvr::SDL2::createWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_w, window_h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  if (!window) {
    std::printf("Failed to create window!\n");
    return EXIT_FAILURE;
  }

  osvr::SDL2::GLContext glctx(window.get());
  SDL_GL_SetSwapInterval(1);

  osvr::clientkit::ClientContext ctx("com.osvr.example.SDLOpenGL");
  osvr::clientkit::DisplayConfig display(ctx);

  if (!display.valid()) {
    std::printf("Failed to get display config!\n");
    return EXIT_FAILURE;
  }

  while (!display.checkStartup()) {
    ctx.update();
  }

  glGenTextures(1, &texture);

  std::string file_ext = filename.substr(filename.find_last_of(".") + 1);

  cv::VideoCapture capture;
  if (file_ext == "avi" || file_ext == "mp4" || file_ext == "mpjg" || file_ext == "mov") {
     capture.open(filename);
  } else if (isdigit(filename[0])) {
    capture.open(stoi(filename));
  } 
  if (!capture.isOpened()) { std::printf("Failed to open camera!\n"); return EXIT_FAILURE; }

  cv::Mat capt_frame;
  std::queue<image_data> image_queue;

  capture >> capt_frame;
  if (capt_frame.empty()) { std::printf("Failed to capture a frame!\n"); return EXIT_FAILURE; }

  image_data capt_image;

  std::vector<bbox_t> result_vec;

//capture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
//capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

/**************
*  main loop  *
**************/

  bool is_running = true;

  while (is_running) {

    frame_start = SDL_GetTicks();

    handleEvents(&is_running);

    capture >> capt_frame;
    if (capt_frame.empty()) { std::printf("Video feed has ended\n"); break; }

    capt_image.frame = capt_frame;
    image_queue.push(capt_image);

    if (!image_queue.empty()) {

/*detect objects and draw a box around them*/
      result_vec = detector.detect(image_queue.front().frame);
      drawBoxes(image_queue.front().frame, result_vec, object_names);
//    showConsoleResult(result_vec, object_names);    //uncomment this if you want console feedback

/*update and render video feed*/
      ctx.update();
      cv::flip(image_queue.front().frame, image_queue.front().frame, 0);
      render(display, image_queue.front().frame);
      SDL_GL_SwapWindow(window.get());

      image_queue.pop();
    }

/*slow down to 30FPS if running faster*/
    frame_time = SDL_GetTicks() - frame_start;
    if (FRAME_DELAY > frame_time) { SDL_Delay(FRAME_DELAY - frame_time); }
  }

  SDL_Quit();

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
void handleEvents(bool* running_status) {
  SDL_Event ev;
  while (SDL_PollEvent(&ev) != 0) {
    if (ev.type == SDL_QUIT) {
      *running_status = false;
    }
    else if (ev.type == SDL_KEYDOWN) {
      switch(ev.key.keysym.sym) {
        case SDLK_ESCAPE:
          *running_status = false;
          break;
        default:
          break;
      }
    }
  }
}

void render(osvr::clientkit::DisplayConfig &display, cv::Mat frame) {
  display.forEachEye([&frame](osvr::clientkit::Eye eye) {
    eye.forEachSurface([&frame](osvr::clientkit::Surface surface) {
      auto viewport = surface.getRelativeViewport();
      glViewport(static_cast<GLint>(viewport.left),
                 static_cast<GLint>(viewport.bottom),
                 static_cast<GLsizei>(viewport.width),
                 static_cast<GLsizei>(viewport.height));
      glLoadIdentity();
      drawToHMD(frame);
    });
  });
}

void drawToHMD(cv::Mat img) {
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.size().width, img.size().height, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);

  glEnable(GL_TEXTURE_2D);

  glBegin(GL_QUADS);
    glTexCoord2d(0.0, 1.0);
    glVertex2d(-1, 1);
    glTexCoord2d(0.0, 0.0);
    glVertex2d(-1, -1);
    glTexCoord2d(1.0, 0.0);
    glVertex2d(1, -1);
    glTexCoord2d(1.0, 1.0);
    glVertex2d(1, 1);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glDeleteTextures(1, &texture);
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

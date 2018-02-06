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
#include <fstream>

#include <osvr/ClientKit/ClientKit.h>
#include <osvr/ClientKit/Display.h>

#include "SDL2Helpers.h"

#include "yolo_v2_class.hpp"

GLuint texture;
void render(osvr::clientkit::DisplayConfig &display, cv::Mat frame);
void drawToHMD(cv::Mat img);
void drawBoxes(cv::Mat matIMG, std::vector<bbox_t> resultVec, std::vector<std::string> objectNames);
void showConsoleResult(std::vector<bbox_t> const resultVec, std::vector<std::string> const objectNames);
cv::Scalar objectIdToColor(int obj_id);
std::vector<std::string> objectNamesFromFile(std::string const filename);
static void handleEvents(bool* runningStatus);

/********************
*                   *
*   MAIN FUNCTION   *
*                   *
********************/

int main(int argc, char* argv[]) {

  std::string namesFile = "darknet/data/voc.names";
  std::string cfgFile = "darknet/cfg/tiny-yolo-voc.cfg";
  std::string weightsFile = "darknet/tiny-yolo-voc.weights";
//std::string cfgFile = "darknet/cfg/yolo-voc.cfg";
//std::string weightsFile = "darknet/yolo-voc.weights";

  Detector detector(cfgFile, weightsFile);

  auto objectNames = objectNamesFromFile(namesFile);

/*for brevity*/
  const auto CENTERED = SDL_WINDOWPOS_CENTERED;

/*used for limiting clock speed*/
  const int FPS = 30;
  const int FRAMEDELAY = 1000 / FPS;
  int frameStart;
  int frameTime;

/*used for on-screen time*/
  time_t rawtime;
  struct tm* timeinfo;
  char timeText[10];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(timeText, 80, "%H:%M:%S", timeinfo);

/*used to determine whether we'll use internal or external webcam*/
  bool externalCam = 0;

/*screen is set to fullscreen by default*/
  bool fullscreen = 1;
  int window_h = 1080;
  int window_w = 1920;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i],"-w") == 0) { //check if windowed mode is wanted
      fullscreen = 0;
      window_h = 504; //16:9
      window_w = 896;
    }
    if (strcmp(argv[i], "-e") == 0) {
      externalCam = 1;
      std::printf("Camera feed taken from external camera\n");
    }
  }

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

  cv::VideoCapture capture;
  capture.open(externalCam);
  if (!capture.isOpened()) { return EXIT_FAILURE; }

  cv::Mat curFrame;
  capture >> curFrame;

  std::vector<bbox_t> resultVec;

//capture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
//capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

/**************
*  main loop  *
**************/

  bool isRunning = true;

  while (isRunning) {

    frameStart = SDL_GetTicks();

/*update clock*/
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeText, 80, "%H:%M:%S", timeinfo);

    handleEvents(&isRunning);

    capture >> curFrame;

    if (!curFrame.empty()) {

/*detect objects and draw a box around them*/
      resultVec = detector.detect(curFrame);
      drawBoxes(curFrame, resultVec, objectNames);
//    showConsoleResult(resultVec, objectNames);    //uncomment this if you want console feedback

/*update and render video feed*/
      ctx.update();
      cv::flip(curFrame, curFrame, 0);
      render(display, curFrame);
      SDL_GL_SwapWindow(window.get());
    }

/*slow down to 30FPS if running faster*/
    frameTime = SDL_GetTicks() - frameStart;
    if (FRAMEDELAY > frameTime) { SDL_Delay(FRAMEDELAY - frameTime); }
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
void handleEvents(bool* runningStatus) {
  SDL_Event ev;
  while (SDL_PollEvent(&ev) != 0) {
    if (ev.type == SDL_QUIT) {
      *runningStatus = false;
    }
    else if (ev.type == SDL_KEYDOWN) {
      switch(ev.key.keysym.sym) {
        case SDLK_ESCAPE:
          *runningStatus = false;
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

void drawBoxes(cv::Mat matIMG, std::vector<bbox_t> resultVec, std::vector<std::string> objectNames) {
  int const colors[6][3] = { {1,0,1},{0,0,1},{0,1,1},{0,1,0},{1,1,0},{1,0,0} };
  for (auto &i : resultVec) {
    cv::Scalar color = objectIdToColor(i.obj_id);
    cv::rectangle(matIMG, cv::Rect(i.x, i.y, i.w, i.h), color, 5);
    if (objectNames.size() > i.obj_id) {
      std::string obj_name = objectNames[i.obj_id];
      if (i.track_id > 0) { obj_name += " - " + std::to_string(i.track_id); }
      cv::Size const text_size = getTextSize(obj_name, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, 2, 0);
      int const max_width = (text_size.width > i.w + 2) ? text_size.width : (i.w + 2);
      cv::rectangle(matIMG, cv::Point2f(std::max((int)i.x - 3, 0), std::max((int)i.y - 30, 0)), cv::Point2f(std::min((int)i.x + max_width, matIMG.cols - 1), std::min((int)i.y, matIMG.rows - 1)), color, CV_FILLED, 8, 0);
      putText(matIMG, obj_name, cv::Point2f(i.x, i.y -10), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, cv::Scalar(0,0,0), 2);
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

void showConsoleResult(std::vector<bbox_t> const resultVec, std::vector<std::string> const objectNames) {
  for (auto &i : resultVec) {
    if (objectNames.size() > i.obj_id) { std::cout << objectNames[i.obj_id] << " - "; }
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

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_opengl.h>
//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/freeglut.h>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>

#include <osvr/ClientKit/ClientKit.h>
#include <osvr/ClientKit/Display.h>

#include "SDL2Helpers.h"


cv::VideoCapture capture;
cv::Mat frame;
GLuint texture;
void render(osvr::clientkit::DisplayConfig &display);
void draw(cv::Mat img);
static void initialization();
static void handleEvents(bool* runningStatus);

/********************
*                   *
*   MAIN FUNCTION   *
*                   *
********************/

int main(int argc, char* argv[]) {

  initialization();

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

  cv::Mat frame;
  capture >> frame;

  capture.open(externalCam);

  if (!capture.isOpened()) {return EXIT_FAILURE;}

  //capture.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
  //capture.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

/**************
*  main loop  *
**************/

  bool isRunning = true;

  while (isRunning) {

    frameStart = SDL_GetTicks();

    handleEvents(&isRunning);

/*update clock*/
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeText, 80, "%H:%M:%S", timeinfo);

/*update and render video feed*/
    ctx.update();
    render(display);
    SDL_GL_SwapWindow(window.get());

/*slow down to 30FPS if running too fast*/
    frameTime = SDL_GetTicks() - frameStart;
    if (FRAMEDELAY > frameTime) {
      SDL_Delay(FRAMEDELAY - frameTime);
    }
  }

  TTF_Quit();
  IMG_Quit();
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

void initialization() {
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  const int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
  if (IMG_Init(imgFlags) != imgFlags) {
    std::fprintf(stderr, "Error: %s", IMG_GetError());
  }
  std::printf("Initialization succesful\n");
}

/*keyboard input*/
void handleEvents(bool* runningStatus) {
  SDL_Event ev;
  while (SDL_PollEvent(&ev) != 0) {
    if (ev.type == SDL_QUIT) {
      *runningStatus = false;
    }
    else if(ev.type == SDL_KEYDOWN) {
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

void render(osvr::clientkit::DisplayConfig &display) {
  display.forEachEye([](osvr::clientkit::Eye eye) {
    eye.forEachSurface([](osvr::clientkit::Surface surface) {
      auto viewport = surface.getRelativeViewport();
      glViewport(static_cast<GLint>(viewport.left),
                 static_cast<GLint>(viewport.bottom),
                 static_cast<GLsizei>(viewport.width),
                 static_cast<GLsizei>(viewport.height));

      glLoadIdentity();

      capture >> frame;

      cv::flip(frame, frame, 0);

      draw(frame);
    });
  });
}

void draw(cv::Mat img) {
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

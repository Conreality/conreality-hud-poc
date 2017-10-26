
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <opencv2/opencv.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>

static void initialization();
static void handleEvents(bool* runningStatus);
SDL_Texture* nextFrameAsTexture(SDL_Renderer* renderer, CvCapture* capture, bool externalCam);

/********************
*                   *
*   MAIN FUNCTION   *
*                   *
********************/

int main(int argc, char* argv[]) {

  initialization();

/*for brevity*/
  const auto CENTERED = SDL_WINDOWPOS_CENTERED;

/*default window size values, 16:9*/
  int window_h = 504;
  int window_w = 896;

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

/*check command line flags for fullscreen*/
  bool fullscreen = 0;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i],"-f") == 0) {
      fullscreen = 1;
      SDL_DisplayMode current;
      SDL_GetCurrentDisplayMode(0, &current); // obtain native resolution
      window_h = current.h;
      window_w = current.w;
      SDL_Log("Current display mode is %dx%dpx", current.w, current.h);
    }
    if (strcmp(argv[i], "-e") == 0) {
      externalCam = 1;
      std::printf("Camera feed taken from external camera\n");
    }
  }

/*create necessary equipment*/
  SDL_Window* window;
  SDL_Texture* imageSurface;
  SDL_Renderer* renderer;

/*window to do everything on*/
  window = SDL_CreateWindow("SDL Window", CENTERED, CENTERED, window_w, window_h, fullscreen);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

/*used for drawing text, such as the clock*/
  TTF_Font* font = TTF_OpenFont("assets/OpenSans-Regular.ttf", 20);
  const SDL_Color greenColor = {0, 255, 0, 255};
  SDL_Surface* textSurface = TTF_RenderText_Solid(font, timeText, greenColor);
  SDL_Texture* clockTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

/*screenCorner is for placement of mentioned on-screen clock*/
  SDL_Rect screenCorner;
  screenCorner.x = screenCorner.y = 0;
  SDL_QueryTexture(clockTexture, nullptr, nullptr, &screenCorner.w, &screenCorner.h); //get width and height of text
  screenCorner.x = (window_w - screenCorner.w -10);
  screenCorner.y = (window_h - screenCorner.h);

/*video feed from webcam*/
  CvCapture* capture = 0;
  capture = cvCaptureFromCAM(externalCam); 

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

/*draw another texture for clock*/
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, timeText, greenColor);
    SDL_Texture* clockTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

/*next frame from webcam*/
    SDL_Texture* frameTexture = nextFrameAsTexture(renderer, capture, fullscreen);

/*render it all unto screen*/
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, frameTexture, nullptr, nullptr);
    SDL_RenderCopy(renderer, clockTexture, nullptr, &screenCorner);
    SDL_RenderPresent(renderer);

/*free used surfaces and textures*/
    SDL_FreeSurface(textSurface), textSurface = nullptr;
    SDL_DestroyTexture(clockTexture), clockTexture = nullptr;
    SDL_DestroyTexture(frameTexture), frameTexture = nullptr;

/*slow down to 30FPS if running too fast*/
    frameTime = SDL_GetTicks() - frameStart;
    if (FRAMEDELAY > frameTime) {
      SDL_Delay(FRAMEDELAY - frameTime);
    }
  }

/*cleaning*/
  SDL_DestroyWindow(window), window = nullptr;
  SDL_DestroyTexture(imageSurface), imageSurface = nullptr;
  SDL_DestroyRenderer(renderer), renderer = nullptr;

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

/*get next frame from webcam*/
SDL_Texture* nextFrameAsTexture(SDL_Renderer* renderer, CvCapture* capture, bool externalCam) {

  IplImage* opencvimg = cvQueryFrame(capture);

/*convert IplImage to an SDL_Surface, which is converted to an SDL_Texture*/
  SDL_Surface* frameSurface = SDL_CreateRGBSurfaceFrom(
            (void*)opencvimg->imageData, opencvimg->width,
             opencvimg->height, opencvimg->depth*opencvimg->nChannels,
             opencvimg->widthStep, 0xff0000, 0x00ff00, 0x0000ff, 0);

  SDL_Texture* frameTexture = SDL_CreateTextureFromSurface(renderer, frameSurface);
  
  SDL_FreeSurface(frameSurface), frameSurface = nullptr;

  return frameTexture;
}

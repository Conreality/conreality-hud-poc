
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>


void initialization();
void handleEvents(bool *runningStatus);


/********************
*                   *
*   MAIN FUNCTION   *
*                   *
********************/

int main (int argc, char *argv[]) {

  initialization();

//for brevity
  const int CENTERED = SDL_WINDOWPOS_CENTERED;  

//default window size values, 16:9
  int WINDOW_H = 504;
  int WINDOW_W = 896;

//used for limiting clock speed
  const int FPS = 30;
  const int frameDelay = 1000 / FPS;
  Uint32 frameStart;
  int frameTime;

//used for on-screen time
  time_t rawtime;
  struct tm *timeinfo;
  char TimeText[10];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime (TimeText, 80, "%H:%M:%S", timeinfo);

//check command line flags for fullscreen 
  bool FULLSCREEN = 0;
  for(int i = 1; i < argc; i++) {
    if(strcmp(argv[i],"-f") == 0) {
      FULLSCREEN = 1;
      SDL_DisplayMode current;
      SDL_GetCurrentDisplayMode(0, &current);     //Obtain native resolution
      WINDOW_H = current.h;
      WINDOW_W = current.w;
      SDL_Log("Current display mode is %dx%dpx", current.w, current.h);
    }
  }

//create necessary equipment
  SDL_Window *window = nullptr;
  SDL_Texture *imageSurface = nullptr;
  SDL_Renderer *renderer = nullptr;

//window to do everything on
  window = SDL_CreateWindow("SDL Window", CENTERED, CENTERED, WINDOW_W, WINDOW_H, FULLSCREEN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

//used for drawing text, such as the clock
  TTF_Font *font = TTF_OpenFont("assets/OpenSans-Regular.ttf", 12);
  SDL_Color color = {0,255,0,255};
  SDL_Surface *textSurface = TTF_RenderText_Solid(font, TimeText, color);
  SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, textSurface);

//screenCorner is for placement of mentioned on-screen clock
  SDL_Rect screenCorner;
  screenCorner.x = screenCorner.y = 0;
  SDL_QueryTexture(text, NULL, NULL, &screenCorner.w, &screenCorner.h);
  screenCorner.x = (WINDOW_W - screenCorner.w -10);
  screenCorner.y = (WINDOW_H - screenCorner.h);


/**************
*  main loop  *
**************/

  bool isRunning = true;

  while(isRunning) {

    frameStart = SDL_GetTicks();
    handleEvents(&isRunning);

//update clock
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(TimeText, 80, "%H:%M:%S", timeinfo);

//draw another texture for clock
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, TimeText, color);
    SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, textSurface);

//render it all unto screen
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, text, NULL, &screenCorner);
    SDL_RenderPresent(renderer);

//free used surfaces and textures
    SDL_FreeSurface(textSurface);
    textSurface = nullptr;
    SDL_DestroyTexture(text);
    text = nullptr;

//slow down to 30fps if running too fast
    frameTime = SDL_GetTicks() - frameStart;
    if(frameDelay > frameTime)
      SDL_Delay(frameDelay - frameTime);

  }

//cleaning
  SDL_DestroyWindow(window);
  SDL_DestroyTexture(imageSurface);
  SDL_DestroyTexture(text);
  SDL_DestroyRenderer(renderer);
  window = nullptr;
  imageSurface = nullptr;
  renderer = nullptr;
  text = nullptr;

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  std::cout << "Program exited" << std::endl;

  return 0;
}


/********************
*                   *
*     FUNCTIONS     *
*                   *
********************/

void initialization() {
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
  if (IMG_Init(imgFlags) != imgFlags) {
    std::cout << "Error: " << IMG_GetError() << std::endl;
  }
  std::cout << "Initialization successful" << std::endl;
}

//keyboard input
void handleEvents(bool *runningStatus) {
  SDL_Event ev;
  while(SDL_PollEvent(&ev) != 0) {

    if(ev.type == SDL_QUIT) {
      *runningStatus = false;
    } else if(ev.type == SDL_KEYDOWN) {
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

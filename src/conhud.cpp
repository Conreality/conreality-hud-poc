/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "version.h"

#include "globals.h"
#include "rendering.h"
#include "input.h"

#ifndef DISABLE_DARKNET
#include "darknet.h"
#endif

#ifndef DISABLE_LEAPMOTION
#include "leapfuncs.h"
#endif

struct image_data {
  cv::Mat frame;
};

Globals global;
void edgeFilter(cv::Mat* image);
void printHelp();

/********************
*                   *
*   MAIN FUNCTION   *
*                   *
********************/

int main(int argc, char* argv[]) {

  std::string filename = "0";
//std::string names_file = "darknet/data/voc.names";
//std::string cfg_file = "darknet/cfg/tiny-yolo-voc.cfg";
//std::string weights_file = "darknet/tiny-yolo-voc.weights";
//std::string cfg_file = "darknet/cfg/yolo-voc.cfg";
//std::string weights_file = "darknet/yolo-voc.weights";
  std::string names_file = "darknet/data/custom.names";
  std::string cfg_file = "darknet/cfg/yolo-voc.2.0.cfg";
  std::string weights_file = "darknet/yolo-voc_custom.weights";

/*used for limiting clock speed*/
  const int FPS = 30;
  const int FRAME_DELAY = 1000 / FPS;
  auto next_frame = std::chrono::steady_clock::now();

  time_t rawtime;
  struct tm* timeinfo;
  char timeText[10];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(timeText, 80, "%H:%M:%S", timeinfo);

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

#ifndef DISABLE_DARKNET
  Detector detector(cfg_file, weights_file);
  auto object_names = objectNamesFromFile(names_file);
#endif

#ifndef DISABLE_LEAPMOTION
  Leap::Controller leap_controller;
  leap_controller.setPolicy(Leap::Controller::POLICY_OPTIMIZE_HMD);
  leap_controller.enableGesture(Leap::Gesture::TYPE_SWIPE);
  leap_event_listener listener;
  leap_controller.addListener(listener);
#endif

#ifndef DISABLE_OSVR
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

  if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
    global.flags.joystick_connected = true;
    std::printf("Joystick detected\n");
  }

  glfwSwapInterval(1);

  glViewport(0, 0, window_w, window_h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, window_w, window_h, 0.0, 0.0, 100.0);
  glMatrixMode(GL_MODELVIEW);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearDepth(0.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

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

  cv::Mat capt_frame, gray_image, contours;

  capture >> capt_frame;
  if (capt_frame.empty()) { std::printf("Failed to capture a frame!\n"); return EXIT_FAILURE; }
  cv::Size const frame_size = capt_frame.size();

  image_data capt_image, pros_image;

  tbb::concurrent_bounded_queue<image_data> image_queue;
  image_queue.set_capacity(10);

  global.kb_control_queue.set_capacity(5);
  global.ms_control_queue.set_capacity(5);

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

#ifndef DISABLE_DARKNET
/*detect objects and draw a box around them*/
      std::vector<bbox_t> result_vec = detector.detect(pros_image.frame);
      drawBoxes(pros_image.frame, result_vec, object_names);
//    showConsoleResult(result_vec, object_names);    //uncomment this if you want console feedback
#endif

      if (global.flags.edge_filter) { edgeFilter(&pros_image.frame); }
      if (global.flags.display_time) {
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(timeText, 80, "%H:%M:%S", timeinfo);

        putText(pros_image.frame, timeText, cv::Point(frame_size.width-230,frame_size.height-150), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(0,255,0), 2);
      }
      if (global.flags.display_name) { putText(pros_image.frame, "player1", cv::Point(frame_size.width/2, 50), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(0,255,0), 2); }
      if (global.flags.flip_image) { cv::flip(pros_image.frame, pros_image.frame, 0); }

/*update and render video feed*/
#ifndef DISABLE_OSVR
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
  leap_controller.removeListener(listener);

  std::printf("Program exited\n");

  return EXIT_SUCCESS;
}

/********************
*                   *
*     FUNCTIONS     *
*                   *
********************/

void edgeFilter(cv::Mat* image) {

  cv::Mat src_gray, canny_output;
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cvtColor(*image, src_gray, CV_BGR2GRAY);
  cv::blur(src_gray, src_gray, cv::Size(3,3));

  cv::Canny(src_gray, canny_output, 30, 120, 3);

  cv::findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0,0));

  if (global.flags.edge_filter_ext) { *image = cv::Scalar(0,0,0); }

  for (unsigned int i = 0; i < contours.size(); i++) {
    cv::Scalar color = cv::Scalar(0,255,0);
    cv::drawContours(*image, contours, i, color, 1, 8, hierarchy, 0, cv::Point());
  }
}

void printHelp() {
  std::printf("help\n");
}

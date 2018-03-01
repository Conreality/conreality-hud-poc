/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "globals.h"
#include "darknet.h"

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

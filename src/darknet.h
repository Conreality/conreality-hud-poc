/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef DARKNET_H
#define DARKNET_H

void drawBoxes(cv::Mat mat_img, std::vector<bbox_t> result_vec, std::vector<std::string> object_names);
void showConsoleResult(std::vector<bbox_t> const result_vec, std::vector<std::string> const object_names);
cv::Scalar objectIdToColor(int obj_id);
std::vector<std::string> objectNamesFromFile(std::string const filename);

#endif //DARKNET_H

#pragma once

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

void calculateTextProperties(const int& width, const int& height, const std::string& text, double& fontScale, int& thickness, cv::Point& position);
void addTextToImage(std::vector<unsigned char>& image_data, const int& width, const int& height, const std::string& text);
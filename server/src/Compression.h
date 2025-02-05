#pragma once

#include <vector>

bool decompress(const std::vector<unsigned char>& jpeg_data, std::vector<unsigned char>& image_data, int& width, int& height);
bool compress(std::vector<unsigned char>& image_data, std::vector<unsigned char>& jpeg_data, const int width, const int height);
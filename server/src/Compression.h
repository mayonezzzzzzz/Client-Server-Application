#pragma once

#include <vector>

bool decompress(const std::vector<char>& jpeg_data, std::vector<unsigned char>& image_data, int& width, int& height, int& quality);
bool compress(const std::vector<unsigned char>& image_data, std::vector<char>& jpeg_data, int width, int height, int& quality);
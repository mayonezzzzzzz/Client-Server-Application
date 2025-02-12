#include "ImageProcessor.h"

void addTextToImage(std::vector<unsigned char>& image_data, const int& width, const int& height, const std::string& text) {
    // Конвертация вектора чаров в Mat
    cv::Mat image(height, width, CV_8UC3, image_data.data());
    if (image.empty()) {
        std::cerr << "Error decoding image data\n";
        return;
    }

    std::cout << "Image dimensions: " << image.rows << "x" << image.cols << std::endl;

    std::cout << "First bytes in image data: ";
    auto& first_bytes = std::min(image_data.size(), static_cast<size_t>(10));
    for (size_t i = 0; i < first_bytes; ++i) {
        std::cout << static_cast<int>(image_data[i]) << " ";
    }
    std::cout << std::endl;

    double fontScale;
    int thickness;
    cv::Point position;
    calculateTextProperties(width, height, text, fontScale, thickness, position);

    // Наложение текста
    cv::putText(image, text, position, cv::FONT_HERSHEY_SIMPLEX, fontScale, cv::Scalar(255, 255, 255), thickness);

    std::cout << "After putText - First bytes in image data: ";
    for (size_t i = 0; i < first_bytes; ++i) {
        std::cout << static_cast<int>(*(image.datastart + i)) << " ";
    }
    std::cout << std::endl;
}

void calculateTextProperties(const int& width, const int& height, const std::string& text, double& fontScale, int& thickness, cv::Point& position) {
    fontScale = width / 1000.0;

    thickness = (width + height) / 1000;

    int baseline = 0;
    cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseline);
    position = cv::Point((width - textSize.width) / 2, height - textSize.height - 10); // смещение на 10 пикселей от нижнего края 
}
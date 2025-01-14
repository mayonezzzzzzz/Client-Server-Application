#include <iostream>
#include <jpeglib.h>
#include "Compression.h"

// декомпресси€
bool decompress(const std::vector<char>& jpeg_data, std::vector<unsigned char>& image_data, int& width, int& height, int& quality) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // ввод данных
    jpeg_mem_src(&cinfo, reinterpret_cast<const unsigned char*>(jpeg_data.data()), jpeg_data.size());

    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
        std::cerr << "Error reading JPEG header." << std::endl;
        return false;
    }
    width = cinfo.image_width;
    height = cinfo.image_height;
    // потенциально - функци€ дл€ сохранени€ качества (степени сжати€) изображени€, использу€ таблицы квантовани€
    //quality = quality_from_quantization_tables(cinfo.quant_tbl_ptrs);

    jpeg_start_decompress(&cinfo);

    int row_stride = cinfo.output_width * cinfo.output_components;
    image_data.resize(row_stride * cinfo.output_height);

    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned char* row_pointer = &image_data[cinfo.output_scanline * row_stride];
        jpeg_read_scanlines(&cinfo, &row_pointer, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return true;
}

// компресси€
bool compress(const std::vector<unsigned char>& image_data, std::vector<char>& jpeg_data, int width, int height, int& quality) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    //запись данных в пам€ть
    unsigned char* jpeg_buffer = nullptr;
    unsigned long jpeg_size = 0;
    jpeg_mem_dest(&cinfo, &jpeg_buffer, &jpeg_size);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3; // RGB
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    while (cinfo.next_scanline < cinfo.image_height) {
        unsigned char* row_pointer = const_cast<unsigned char*>(&image_data[cinfo.next_scanline * width * 3]);
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);

    //копируем результат в выходной вектор
    jpeg_data.assign(reinterpret_cast<char*>(jpeg_buffer), reinterpret_cast<char*>(jpeg_buffer) + jpeg_size);

    jpeg_destroy_compress(&cinfo);
    return true;
}
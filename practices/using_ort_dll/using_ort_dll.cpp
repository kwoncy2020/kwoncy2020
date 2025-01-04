#define MyTimeCheck

#ifdef MyTimeCheck
#include <chrono>
#include <iostream>
// https://m.blog.naver.com/dorergiverny/223052685676
#define _MEASURE_START(STR)	std::chrono::system_clock::time_point start_##STR = std::chrono::system_clock::now();
#define _MEASURE_END(STR, loop)	std::chrono::duration<double, std::milli> duration_msec_##STR = std::chrono::system_clock::now() - start_##STR;\
							std::cout << ">>>>> [" << #STR << "]\t" << loop << " loop average : "<< duration_msec_##STR.count() / loop << " msec\n";

#endif


// #if defined(_WIN32) && defined(STBIW_WINDOWS_UTF8)
// #define STBI_NEON   // for android

#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"


int main(){
    std::string file_name = "11_0_test.png";
    // std::string replace_string = "_resized.jpg";
    // const char* file_name{"11_0_test.png"};
    int image_w_orig, image_h_orig, image_comp_orig;
    unsigned char* image_data = stbi_load(file_name.c_str(),&image_w_orig,&image_h_orig,&image_comp_orig,0);

    int image_w_resized=512;
    int image_h_resized=512;
    unsigned char* image_data_resized = stbir_resize_uint8_linear(image_data, image_w_orig, image_h_orig, image_w_orig*image_comp_orig,nullptr,image_w_resized,image_h_resized,image_w_resized*image_comp_orig,STBIR_RGB);
    stbi_write_jpg(file_name.replace(file_name.end()-4,file_name.end(),"_resized.jpg").c_str(),image_w_resized,image_h_resized,image_comp_orig,image_data_resized,80);



    return 0;
}
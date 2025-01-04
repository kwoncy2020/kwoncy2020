
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
extern "C"{
void __declspec(dllexport) tflite_c(
    unsigned char* buffer_func_input, int byteSize_input,
    const char* model_name_input, int num_threads_input, int model_bits_input, 
    bool is_model_qat_input, bool use_xnn_input, bool use_gpu_input, bool use_npu_input, 
    int loop_count_time_input);

}
// int __declspec(dllexport) stbi_write_jpg(char const *filename, int w, int h, int comp, const void *data, int quality);;

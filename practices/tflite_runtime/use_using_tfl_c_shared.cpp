#include "using_tfl_c_shared.h"

int main(){

    const char* save_image_name = "use_using_tfl_c_shared.jpg";
    unsigned char* buffer = (unsigned char*)malloc(512*512*8);
    // tflite_c(buffer, byteSize, model_name, num_threads, model_bits, is_model_qat
    //          use_xnn, use_gpu, use_npu);
    tflite_c(buffer,512*512*3,nullptr,4,8,false,/*use_xnn*/true,false,false,5);
    // tflite_c(buffer,512*512*3,nullptr,4,8,false,/*use_xnn*/false,false,false,5);
    // tflite_c(buffer,512*512*3,nullptr,4,32,false,/*use_xnn*/true,false,false,5);
    // tflite_c(buffer,512*512*3,nullptr,4,32,false,/*use_xnn*/false,false,false,5);
    stbi_write_jpg(save_image_name, 512, 512, 3, buffer, 100);
    free(buffer);
    return 0;
}

#define MyFileCheck
#define MyTimeCheck

#include "tensorflow/lite/core/c/c_api.h"

#ifdef MyFileCheck
#include <fstream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
// https://m.blog.naver.com/dorergiverny/223052685676
#define _MEASURE_START(STR)	std::chrono::system_clock::time_point start_##STR = std::chrono::system_clock::now();
#define _MEASURE_END(STR, loop)	std::chrono::duration<double, std::milli> duration_msec_##STR = std::chrono::system_clock::now() - start_##STR;\
							std::cout << ">>>>> [" << #STR << "]\t" << loop << " loop average : "<< duration_msec_##STR.count() / loop << " msec\n";

#endif

#ifdef MyTimeCheck
#include <chrono>
#include <iostream>
#endif

int main()
{
    int length = 0;
    unsigned char *buffer;
    unsigned char *buffer_out;
    unsigned char *buffer_file_out;
    float* input_buffer_float;
    float* output_buffer_float;
    const char *input_binary_name;
    const char *model_name;
    const char *output_file_name;

    int32_t num_threads = 8;
    int model_bits = 32;
    // bool ismodel_QAT = false;
    bool ismodel_QAT = true;
    TfLiteStatus status;


    if(model_bits==8){
        input_binary_name = "binary_11_0_";
        // const char *input_binary_name = "binary_132_0_";
        if (!ismodel_QAT){
            model_name = "zdce++enhanced_out_okv3_f_int_q_uint8.tflite";
            output_file_name = "out_binary.jpg";
        }else{
            model_name = "zero_dce_qat_okv3_f_int_q_uint8.tflite";
            output_file_name = "out_binary_qat.jpg";
        }

    }
    else{
        input_binary_name = "binary_11_0_float";
        // const char *input_binary_name = "binary_132_0_";
        if (!ismodel_QAT){
            model_name = "zdce++enhanced_out_float32.tflite";
            output_file_name = "out_binary_float.jpg";
        }else{
            model_name = "zero_dce_qat_float32.tflite";
            output_file_name = "out_binary_float_qat.jpg";
        }
    }

    int h = 512;
    int w = 512;

    std::cout << "num_threads : " << num_threads << std::endl;
    std::cout << "num_model_bits : " << model_bits << std::endl;
    std::cout << "loaded tflite model name : " << model_name << std::endl;
    std::cout << "input_binary_name : " << input_binary_name << std::endl;
    std::cout << "output_file_name : " << output_file_name << std::endl;


    std::ifstream is(input_binary_name, std::ifstream::binary);
    if (is.is_open())
    {
        is.seekg(0, is.end);
        length = (int)is.tellg();
        is.seekg(0, is.beg);

        buffer = (unsigned char *)malloc(length);
        buffer_out = (unsigned char *)malloc(length);
        is.read((char *)buffer, length);
        is.close();
    }

    if(model_bits==32){
        input_buffer_float = (float *)buffer;
        output_buffer_float = (float *)buffer_out;
        // for (size_t i = 0; i < (length/4); i++)
        // {
        //     *(int_buffer+i) *= 255.0;
        // }

    }

#ifdef MyTimeCheck
    // std::chrono::system_clock::time_point model_load_and_prepare_time_start = std::chrono::system_clock::now();
    _MEASURE_START(model_load_and_prepare_time)
#endif

    TfLiteModel *model = TfLiteModelCreateFromFile(model_name);

    TfLiteInterpreterOptions *options = TfLiteInterpreterOptionsCreate();
    TfLiteInterpreterOptionsSetNumThreads(options, num_threads);

    TfLiteInterpreter *interpreter = TfLiteInterpreterCreate(model, options);

    status = TfLiteInterpreterAllocateTensors(interpreter);
    TfLiteTensor *input_tensor = TfLiteInterpreterGetInputTensor(interpreter, 0);
    if(model_bits==8)
        status = TfLiteTensorCopyFromBuffer(input_tensor, buffer, length);
    // TfLiteTensorCopyFromBuffer(input_tensor, buffer, length);
    else{
        // status = TfLiteTensorCopyFromBuffer(input_tensor, buffer, length);
        // TfLiteTensorCopyFromBuffer(input_tensor, (float *)buffer, length/4);
        status = TfLiteTensorCopyFromBuffer(input_tensor, input_buffer_float, length);
    }

#ifdef MyTimeCheck
    // std::chrono::system_clock::time_point model_load_and_prepare_time_end = std::chrono::system_clock::now();
    // std::chrono::microseconds model_load_and_prepare_time = std::chrono::duration_cast<std::chrono::microseconds>(model_load_and_prepare_time_end-model_load_and_prepare_time_start);
    // std::cout << "model_load_and_prepare_time : " << model_load_and_prepare_time << "usec" << std::endl;
    _MEASURE_END(model_load_and_prepare_time,1)
#endif

    // warmup
    status = TfLiteInterpreterInvoke(interpreter);

#ifdef MyTimeCheck
    // std::chrono::system_clock::time_point model_inference_time_start = std::chrono::system_clock::now();
    _MEASURE_START(model_inference_time)
#endif


#ifdef MyTimeCheck
    int loop_count = 1;
    for (size_t i = 0; i < loop_count; i++)
    {
        TfLiteInterpreterInvoke(interpreter);
    }
    
    _MEASURE_END(model_inference_time,loop_count)
#endif


    // int input_num = TfLiteInterpreterGetInputTensorCount(interpreter);
    // int out_num = TfLiteInterpreterGetOutputTensorCount(interpreter);
    const TfLiteTensor *output_tensor = TfLiteInterpreterGetOutputTensor(interpreter, 0);
    if(model_bits==8)
        status = TfLiteTensorCopyToBuffer(output_tensor, buffer_out, length);
    // TfLiteTensorCopyToBuffer(output_tensor, buffer, length);
    else{
        status = TfLiteTensorCopyToBuffer(output_tensor, buffer_out, length);
        // TfLiteTensorCopyFromBuffer(input_tensor, (float *)buffer_out, length/4);
        // status = TfLiteTensorCopyToBuffer(output_tensor, output_buffer_float, length);
    
    
    
        // std::cout << 50 << " st : "<<*(output_buffer_float + 50) << std::endl;
        // std::cout << 100000 << " st : "<<*(output_buffer_float + 100000) << std::endl;
        // std::cout << 200000 << " st : "<<*(output_buffer_float + 200000) << std::endl;
        // std::cout << 300000 << " st : "<<*(output_buffer_float + 300000) << std::endl;
        // std::cout << 700000 << " st : "<<*(output_buffer_float + 700000) << std::endl;
        // std::cout << 780000 << " st : "<<*(output_buffer_float + 780000) << std::endl;

        buffer_file_out = (unsigned char *)malloc(length/4);
        for (size_t i = 0; i < length/4; i++)
        {
            // *(unsigned char)(output_buffer_float+i)=(unsigned char)(*(output_buffer_float+i)*255);
            *(buffer_file_out+i) = (unsigned char)(*(output_buffer_float+i)*255);
        }

        // std::cout << 50 << " st : "<<*(buffer_file_out + 50) << std::endl;
        // std::cout << 100000 << " st : "<<*(buffer_file_out + 100000) << std::endl;
        // std::cout << 200000 << " st : "<<*(buffer_file_out + 200000) << std::endl;
        // std::cout << 300000 << " st : "<<*(buffer_file_out + 300000) << std::endl;
        // std::cout << 700000 << " st : "<<*(buffer_file_out + 700000) << std::endl;
        // std::cout << 780000 << " st : "<<*(buffer_file_out + 780000) << std::endl;


    }


    
    

#ifdef MyFileCheck
    if(model_bits==8)
        stbi_write_jpg(output_file_name, w, h, 3, buffer_out, 100);
    else{
        // stbi_write_jpg(output_file_name, w, h, 3, buffer_out_float, 100);
        // stbi_write_jpg(output_file_name, w, h, 3, (uint8_t*)output_buffer_float, 100);
        stbi_write_jpg(output_file_name, w, h, 3, buffer_file_out, 100);

    }
    // std::ofstream out;
    // out.open("out.bmp", std::ofstream::binary);
    // out.write((const char *)buffer_out, length);
    // out.close();
#endif

    TfLiteInterpreterDelete(interpreter);
    TfLiteInterpreterOptionsDelete(options);
    TfLiteModelDelete(model);

    free(buffer);
    free(buffer_out);
    if(model_bits==32){
        free(buffer_file_out);
    }

    return 0;
}

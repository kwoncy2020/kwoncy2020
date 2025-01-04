// #define MyFileCheck
#define MyTimeCheck

#include "tensorflow/lite/core/c/c_api.h"
#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/delegates/xnnpack/xnnpack_delegate.h"
#include "flutter_my_ai.h"

#ifdef MyFileCheck
#include <fstream>
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb_image_write.h"
#endif

#ifdef MyTimeCheck
#include <chrono>
#include <iostream>
// https://m.blog.naver.com/dorergiverny/223052685676
#define _MEASURE_START(STR)	std::chrono::system_clock::time_point start_##STR = std::chrono::system_clock::now();
#define _MEASURE_END(STR, loop)	std::chrono::duration<double, std::milli> duration_msec_##STR = std::chrono::system_clock::now() - start_##STR;\
							std::cout << ">>>>> [" << #STR << "]\t" << (loop) << " loop average : "<< duration_msec_##STR.count() / (loop) << " msec\n";

#endif

// #include <windows.h>


void tflite_c(unsigned char* buffer_func_input, const char* model_name_input,int byteSize_input, int num_threads_input=4, int model_bits_input=8, bool is_model_qat_input=false, bool use_xnn_input=false, bool use_gpu_input=false, bool use_npu_input=false, int loop_count_time_input=1)
{
    if(model_bits_input != 8 && model_bits_input != 32){
        std::cout << "wrong model_bits (must be 8 or 32) : " << model_bits_input << std::endl;
        return ;
    }
    if (model_bits_input == 8 && byteSize_input!=512*512*3)
        std::cout << "model_bits_input = 8, input_height=512, input_width=512, byteSize shoud be 512*512*3 : " << 512*512*3 << " but received : " << byteSize_input;
        // printf("model_bits_input = 8, input_height=512, input_width=512, byteSize shoud be 512*512*3 : %d but received : %d",512*512*3, byteSize_input);
    if (model_bits_input == 32 && byteSize_input!=512*512*3*4)
        std::cout << "model_bits_input = 32, input_height=512, input_width=512, byteSize shoud be 512*512*3*4 : " << 512*512*3*4 << " but received : " << byteSize_input;
        // printf("model_bits_input = 32, input_height=512, input_width=512, byteSize shoud be 512*512*3*4 : %d but received : %d",512*512*3*4, byteSize_input);

    // unsigned char *buffer_out=nullptr;
    int32_t num_threads = num_threads_input;
    int model_bits = model_bits_input;
    bool ismodel_QAT = is_model_qat_input;
    // bool ismodel_QAT = true;  //8bit QAT produce wrong output. there was wrong quantization.(float to 8bit simple quant)
    bool use_xnn = use_xnn_input;
    bool use_gpu = use_gpu_input;
    bool use_npu = use_npu_input;
    int loop_count_time = loop_count_time_input;

    TfLiteDelegate* xnnDelegate=nullptr;
    TfLiteStatus status;
    
    int h = 512;
    int w = 512;

    std::cout << "num_threads : " << num_threads << std::endl;
    std::cout << "num_model_bits : " << model_bits << std::endl;
    std::cout << "tflite model name will be loaded: " << model_name_input << std::endl;

#ifdef MyTimeCheck
    // std::chrono::system_clock::time_point model_load_and_prepare_time_start = std::chrono::system_clock::now();
    _MEASURE_START(model_load_and_prepare_time)
#endif

    TfLiteModel *model = TfLiteModelCreateFromFile(model_name_input);
    std::cout << "model loaded from file." << std::endl;
    TfLiteInterpreterOptions *options = TfLiteInterpreterOptionsCreate();
    TfLiteInterpreterOptionsSetNumThreads(options, num_threads);

    if (use_xnn){
        TfLiteXNNPackDelegateOptions xnndelegateOptions = TfLiteXNNPackDelegateOptionsDefault();
        if (model_bits == 8){
            xnndelegateOptions.flags |= TFLITE_XNNPACK_DELEGATE_FLAG_QS8;
            xnndelegateOptions.flags |= TFLITE_XNNPACK_DELEGATE_FLAG_QU8;
        }
        xnnDelegate = TfLiteXNNPackDelegateCreate(&xnndelegateOptions);
        // TfLiteGpuDelegateV2Create();
        TfLiteInterpreterOptionsAddDelegate(options, xnnDelegate);
        std::cout << "xnnpack delegate added." << std::endl;
    }
    
    TfLiteInterpreter *interpreter = TfLiteInterpreterCreate(model, options);
    std::cout << "interpreter created." << std::endl;

    status = TfLiteInterpreterAllocateTensors(interpreter);
    TfLiteTensor *input_tensor = TfLiteInterpreterGetInputTensor(interpreter, 0);
    status = TfLiteTensorCopyFromBuffer(input_tensor, buffer_func_input, byteSize_input);

#ifdef MyTimeCheck
    // std::chrono::system_clock::time_point model_load_and_prepare_time_end = std::chrono::system_clock::now();
    // std::chrono::microseconds model_load_and_prepare_time = std::chrono::duration_cast<std::chrono::microseconds>(model_load_and_prepare_time_end-model_load_and_prepare_time_start);
    // std::cout << "model_load_and_prepare_time : " << model_load_and_prepare_time << "usec" << std::endl;
    _MEASURE_END(model_load_and_prepare_time,1)
#endif
    
#ifdef MyTimeCheck
    // std::chrono::system_clock::time_point model_inference_time_start = std::chrono::system_clock::now();
    _MEASURE_START(model_inference_time_1plus_loop_cout)
#endif

    // warmup
    std::cout << "inference start." << std::endl;
    status = TfLiteInterpreterInvoke(interpreter);

#ifdef MyTimeCheck
    int loop_count = loop_count_time;
    for (size_t i = 0; i < loop_count; i++)
    {
        TfLiteInterpreterInvoke(interpreter);
    }
    
    _MEASURE_END(model_inference_time_1plus_loop_cout,loop_count+1)
#endif

    std::cout << "inference done." << std::endl;

    const TfLiteTensor *output_tensor = TfLiteInterpreterGetOutputTensor(interpreter, 0);
    // status = TfLiteTensorCopyToBuffer(output_tensor, buffer_func_input, byteSize_input);
    std::cout << "start :copy model output tensor buffer to buffer_out." << std::endl;
    // status = TfLiteTensorCopyToBuffer(output_tensor, buffer_out, byteSize_input);
    status = TfLiteTensorCopyToBuffer(output_tensor, buffer_func_input, byteSize_input);
    std::cout << "copy status : " << status << std::endl;

    // std::cout << "copy output buffer to input buffer." << std::endl;
    // memcpy(buffer_func_input,buffer_out,byteSize_input);


    if (use_xnn) TfLiteXNNPackDelegateDelete(xnnDelegate);
    TfLiteInterpreterDelete(interpreter);
    TfLiteInterpreterOptionsDelete(options);
    TfLiteModelDelete(model);
    // free(buffer_out);
    return ;
}

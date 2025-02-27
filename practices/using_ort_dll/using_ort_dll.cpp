#define MyTimeCheck

#ifdef MyTimeCheck
#include <chrono>
#include <iostream>
// https://m.blog.naver.com/dorergiverny/223052685676
#define _MEASURE_START(STR)	std::chrono::system_clock::time_point start_##STR = std::chrono::system_clock::now();
#define _MEASURE_END(STR, loop)	std::chrono::duration<double, std::milli> duration_msec_##STR = std::chrono::system_clock::now() - start_##STR;\
							std::cout << ">>>>> [" << #STR << "]\t" << loop << " loop average : "<< duration_msec_##STR.count() / loop << " msec\n" << "total : " <<duration_msec_##STR.count() << " msec\n";

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

#include "onnxruntime_cxx_api.h"
#include "dml_provider_factory.h"
#include <vector>


bool CheckStatus(const OrtApi* g_ort, OrtStatus* status) {
  if (status != nullptr) {
    const char* msg = g_ort->GetErrorMessage(status);
    std::cerr << msg << std::endl;
    g_ort->ReleaseStatus(status);
    throw Ort::Exception(msg, OrtErrorCode::ORT_EP_FAIL);
  }
  return true;
}


int main(){
    // bool use_xnn=true;
    bool use_xnn=false;
    bool use_dml=true;
    // bool use_dml=false;
    
    int intra_op_num_threads = 4;
    int infer_time_check_loop = 5;
    std::string file_name = "11_0_test.png";
    std::wstring model_path = L"my_modified.onnx";

    std::cout << "intra_op_num_threads : " << intra_op_num_threads << std::endl;
    std::cout << "loaded file_name : " << file_name << std::endl;
    if (use_xnn) std::cout << "use_xnn" << std::endl; 
    if (use_dml) std::cout << "use_dml" << std::endl;

    // std::string replace_string = "_resized.jpg";
    // const char* file_name{"11_0_test.png"};
    int image_w_orig, image_h_orig, image_comp_orig;
    unsigned char* image_data = stbi_load(file_name.c_str(),&image_w_orig,&image_h_orig,&image_comp_orig,0);

    int image_w_resized=512;
    int image_h_resized=512;
    unsigned char* image_data_resized = stbir_resize_uint8_linear(image_data, image_w_orig, image_h_orig, image_w_orig*image_comp_orig,nullptr,image_w_resized,image_h_resized,image_w_resized*image_comp_orig,STBIR_RGB);
    // stbi_write_jpg(file_name.replace(file_name.end()-4,file_name.end(),"_resized.jpg").c_str(),image_w_resized,image_h_resized,image_comp_orig,image_data_resized,80);

#ifdef MyTimeCheck
    _MEASURE_START(prepare_onnxruntime)
#endif

    const OrtApi* g_ort = OrtGetApiBase()->GetApi(ORT_API_VERSION);
    OrtEnv* env;

    CheckStatus(g_ort, g_ort->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "test", &env));

    OrtSessionOptions* session_options;
    CheckStatus(g_ort, g_ort->CreateSessionOptions(&session_options));
    CheckStatus(g_ort, g_ort->SetIntraOpNumThreads(session_options,intra_op_num_threads));
    // CheckStatus(g_ort,g_ort->SetSessionGraphOptimizationLevel(session_options, ORT_ENABLE_BASIC));
    if (use_xnn) {
        std::vector<const char*> option_keys{"intra_op_num_threads"};
        // auto num_thread_str = std::to_string(intra_op_num_threads);
        auto num_thread_str = std::to_string(0);
        // const char* num_threads = a.c_str();
        std::vector<const char*> option_values{num_thread_str.c_str()};
        CheckStatus(g_ort,g_ort->SessionOptionsAppendExecutionProvider(session_options,"XNNPACK",option_keys.data(),option_values.data(),1));
    }

    if (use_dml){
        // CheckStatus(g_ort,g_ort->SessionOption)
        // OrtDmlApi::SessionOptionsAppendExecutionProvider_DML();
        OrtSessionOptionsAppendExecutionProvider_DML(session_options,0);
        CheckStatus(g_ort,g_ort->DisableMemPattern(session_options));
        CheckStatus(g_ort,g_ort->SetSessionExecutionMode(session_options, ExecutionMode::ORT_SEQUENTIAL));
    }

    OrtSession* session;
    CheckStatus(g_ort,g_ort->CreateSession(env, model_path.c_str(), session_options, &session));

    OrtAllocator* allocator;
    CheckStatus(g_ort, g_ort->GetAllocatorWithDefaultOptions(&allocator));
    size_t num_input_nodes;
    CheckStatus(g_ort,g_ort->SessionGetInputCount(session, &num_input_nodes));

    std::vector<const char*> input_node_names;
    std::vector<std::vector<int64_t>> input_node_dims;
    std::vector<ONNXTensorElementDataType> input_types;
    std::vector<OrtValue*> input_tensors;
    
    input_node_names.resize(num_input_nodes);
    input_node_dims.resize(num_input_nodes);
    input_types.resize(num_input_nodes);
    input_tensors.resize(num_input_nodes);

    for (size_t i = 0; i < num_input_nodes; ++i)
    {
        char* input_name;
        CheckStatus(g_ort, g_ort->SessionGetInputName(session, i, allocator, &input_name));
        input_node_names[i] = input_name;

        OrtTypeInfo* type_info;
        CheckStatus(g_ort, g_ort->SessionGetInputTypeInfo(session,i,&type_info));
        const OrtTensorTypeAndShapeInfo* tensor_info;
        CheckStatus(g_ort, g_ort->CastTypeInfoToTensorInfo(type_info, &tensor_info));
        ONNXTensorElementDataType type;
        CheckStatus(g_ort,g_ort->GetTensorElementType(tensor_info, &type));
        input_types[i]=type;

        size_t num_dims;
        CheckStatus(g_ort,g_ort->GetDimensionsCount(tensor_info, &num_dims));
        input_node_dims[i].resize(num_dims);
        CheckStatus(g_ort,g_ort->GetDimensions(tensor_info, input_node_dims[i].data(), num_dims));
        if (type_info) g_ort->ReleaseTypeInfo(type_info);
    }
    
    size_t num_output_nodes;
    std::vector<const char*> output_node_names;
    std::vector<std::vector<int64_t>> output_node_dims;
    std::vector<OrtValue*> output_tensors;
    CheckStatus(g_ort,g_ort->SessionGetOutputCount(session, &num_output_nodes));
    
    output_node_names.resize(num_output_nodes);
    output_node_dims.resize(num_output_nodes);
    output_tensors.resize(num_output_nodes);

    for (size_t i = 0; i < num_output_nodes; ++i)
    {
        char* output_name;
        CheckStatus(g_ort,g_ort->SessionGetOutputName(session,i,allocator,&output_name));
        output_node_names[i] = output_name;

        OrtTypeInfo* type_info;
        CheckStatus(g_ort,g_ort->SessionGetOutputTypeInfo(session,i,&type_info));
        const OrtTensorTypeAndShapeInfo* tensor_info;
        CheckStatus(g_ort,g_ort->CastTypeInfoToTensorInfo(type_info,&tensor_info));

        size_t num_dims;
        CheckStatus(g_ort,g_ort->GetDimensionsCount(tensor_info, &num_dims));
        output_node_dims[i].resize(num_dims);
        CheckStatus(g_ort,g_ort->GetDimensions(tensor_info, (int64_t*)output_node_dims[i].data(),num_dims));

        if(type_info) g_ort->ReleaseTypeInfo(type_info);
    }
    
    OrtMemoryInfo* memory_info;
    size_t input_data_size = 1*512*512*3;
    const size_t num_elements = 1*512*512*3;

    CheckStatus(g_ort, g_ort->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info));
    size_t input_data_length = input_data_size * sizeof(uint8_t);
    CheckStatus(g_ort,g_ort->CreateTensorWithDataAsOrtValue(memory_info, reinterpret_cast<void*>(image_data_resized), input_data_length,
                            input_node_dims[0].data(), input_node_dims[0].size(), input_types[0], &input_tensors[0]));
    
    g_ort->ReleaseMemoryInfo(memory_info);

#ifdef MyTimeCheck
    _MEASURE_END(prepare_onnxruntime,1)
#endif

#ifdef MyTimeCheck
    _MEASURE_START(onnxruntime_session_run)
#endif
    int loop_count=infer_time_check_loop;
    for (size_t i = 0; i < loop_count; ++i)
    {
        CheckStatus(g_ort,g_ort->Run(session,nullptr,input_node_names.data(),(const OrtValue* const*)input_tensors.data(),
                                    input_tensors.size(), output_node_names.data(), output_node_names.size(), output_tensors.data()));
    }

#ifdef MyTimeCheck
    _MEASURE_END(onnxruntime_session_run,loop_count)
    // _MEASURE_END(onnxruntime_session_run,1)
#endif

    // float *output_data = (float*)malloc(1*3*512*512*sizeof(float));
    void* output_buffer;
    CheckStatus(g_ort,g_ort->GetTensorMutableData(output_tensors[0],&output_buffer));
    // float* float_buffer = nullptr;
    // float_buffer = reinterpret_cast<float*>(output_buffer);
    // unsigned char* output_image_data = (unsigned char*)malloc(1*3*512*512*sizeof(uint8_t));
    // for (size_t i = 0; i < 1*3*512*512; ++i)
    // {
    //     *(output_image_data+i) = *(float_buffer+i)*255;
    // }
    
    stbi_write_jpg(file_name.replace(file_name.end()-4,file_name.end(),"_output.jpg").c_str(),image_w_resized,image_h_resized,image_comp_orig,output_buffer,80);



    return 0;
}
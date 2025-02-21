#define MyTimeCheck

#ifdef MyTimeCheck
#include <chrono>
#include <iostream>
// https://m.blog.naver.com/dorergiverny/223052685676
#define _MEASURE_START(STR)	std::chrono::system_clock::time_point start_##STR = std::chrono::system_clock::now();
#define _MEASURE_END(STR, loop)	std::chrono::duration<double, std::milli> duration_msec_##STR = std::chrono::system_clock::now() - start_##STR;\
							std::cout << ">>>>> [" << #STR << "]\t" << loop << " loop average : "<< duration_msec_##STR.count() / loop << " msec\n" << "total : " <<duration_msec_##STR.count() << " msec\n";

#endif

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

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
#include <format>
#include "using_ort_dll_detection_label_mapping.h"
#include <source_location>
#include "using_ort_dll_detection.h"

bool CheckStatus(const OrtApi* g_ort, OrtStatus* status) {
  if (status != nullptr) {
    const char* msg = g_ort->GetErrorMessage(status);
    std::cerr << msg << std::endl;
    g_ort->ReleaseStatus(status);
    throw Ort::Exception(msg, OrtErrorCode::ORT_EP_FAIL);
  }
  return true;
}

std::string makeLastError(std::string_view message, const std::source_location& location=std::source_location::current()){
    std::string ret_{location.function_name()};
    ret_ = std::format("{} : {}", ret_, message);
    return std::move(ret_);
}


ImageDetectionAI::ImageDetectionAI():g_ort(OrtGetApiBase()->GetApi(ORT_API_VERSION)){
        model_h = 640;
        model_w = 640;
        model_c = 3;
        intra_op_num_threads=4;
        infer_time_check_loop=1;
        score_thres = 0.5;
        _execution_provider=EXECUTION_PROVIDER::CPU;
    };


bool ImageDetectionAI::is_model_loaded(){
    return _is_model_loaded;
}

bool ImageDetectionAI::set_execution_provider(const char* provider){
    if (provider == "cpu"){
        _execution_provider = EXECUTION_PROVIDER::CPU;
        return true;
    }
    if (provider == "gpu"){
        _execution_provider = EXECUTION_PROVIDER::GPU;
        return true;
    }
    if (provider == "npu"){
        _execution_provider = EXECUTION_PROVIDER::NPU;
        return true;
    }
    if (provider == "xnnpack"){
        _execution_provider = EXECUTION_PROVIDER::XNNPACK;
        return true;
    }
    
    _last_error = makeLastError("provider setting failed. provider should be (cpu|gpu|dml|npu|xnnpack)");
    return false;
}

const char* ImageDetectionAI::get_last_error(){
    return _last_error.c_str();
}

bool ImageDetectionAI::load_model(const char* model_path){
    _is_model_loaded=false;
    _model_path = std::string(model_path);

    try{
        if(g_ort==nullptr){
            _last_error=makeLastError("model ptr (g_ort) == nullptr");
            return false;
        }
        CheckStatus(g_ort, g_ort->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "test", &env));
        CheckStatus(g_ort, g_ort->CreateSessionOptions(&session_options));
        CheckStatus(g_ort, g_ort->SetIntraOpNumThreads(session_options,intra_op_num_threads));

        switch(_execution_provider){
            case EXECUTION_PROVIDER::XNNPACK:
                {
                std::vector<const char*> option_keys{"intra_op_num_threads"};
                auto num_thread_str = std::to_string(intra_op_num_threads);
                std::vector<const char*> option_values{num_thread_str.c_str()};
                CheckStatus(g_ort,g_ort->SessionOptionsAppendExecutionProvider(session_options,"XNNPACK",option_keys.data(),option_values.data(),1));
                break;
                }
            case EXECUTION_PROVIDER::DML:
                OrtSessionOptionsAppendExecutionProvider_DML(session_options,0);
                CheckStatus(g_ort,g_ort->DisableMemPattern(session_options));
                CheckStatus(g_ort,g_ort->SetSessionExecutionMode(session_options, ExecutionMode::ORT_SEQUENTIAL));
                break;
            
        }

        std::vector<wchar_t> vec;
        size_t len = strlen(model_path);
        vec.resize(len+1);
        mbstowcs(&vec[0],model_path,len);
        const wchar_t* w_model_path = &vec[0];
        
        CheckStatus(g_ort,g_ort->CreateSession(env, w_model_path, session_options, &session));
        CheckStatus(g_ort, g_ort->GetAllocatorWithDefaultOptions(&allocator));
        CheckStatus(g_ort,g_ort->SessionGetInputCount(session, &num_input_nodes));

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

    }catch(std::exception &e){
        _last_error=makeLastError(std::format("exception occured e : {}", e.what()));
    }

    _is_model_loaded=true;
    return true;
}

bool ImageDetectionAI::inference_from_file(const char* file_path){
    if (!_is_model_loaded){
        _last_error = makeLastError("model hasn't been loaded.");
        return false;
    }
    int image_w_orig, image_h_orig, image_comp_orig;
    cv::Mat loaded_image = cv::imread(file_path,1);
    image_w_orig = loaded_image.cols;
    image_h_orig = loaded_image.rows;
    image_comp_orig = loaded_image.dims;
    unsigned char* image_data = loaded_image.data;
    
    int image_w_resized=model_w;
    int image_h_resized=model_h;
    
    cv::Mat resized_image;
    cv::resize(loaded_image,resized_image,cv::Size(model_w,model_h),0,0,cv::INTER_LANCZOS4);
    unsigned char* image_data_resized = resized_image.data;
    int image_c_resized = resized_image.channels();

    int image_data_resized_length = image_w_resized * image_h_resized * image_c_resized * sizeof(uint8_t);

    return inference_from_bytes(image_data_resized, image_data_resized_length);
}

bool ImageDetectionAI::inference_from_bytes(unsigned char* buffer_data, int data_length){
    if (!_is_model_loaded){
        _last_error = makeLastError("model hasn't been loaded.");
        return false;
    }

    OrtMemoryInfo* memory_info;
    size_t input_data_size = 1*model_h*model_w*model_c;
    const size_t num_elements = 1*model_h*model_w*model_c;

    CheckStatus(g_ort, g_ort->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info));
    size_t input_data_length = input_data_size * sizeof(uint8_t);
    if (input_data_length != data_length){
        _last_error = makeLastError(std::format("data length miss match : data_length({}) and input_data_length({})",data_length, input_data_length));
        return false;
    }

    CheckStatus(g_ort,g_ort->CreateTensorWithDataAsOrtValue(memory_info, reinterpret_cast<void*>(buffer_data), input_data_length,
                            input_node_dims[0].data(), input_node_dims[0].size(), input_types[0], &input_tensors[0]));
    std::vector<int64_t> input1{model_h,model_w};
    CheckStatus(g_ort,g_ort->CreateTensorWithDataAsOrtValue(memory_info,reinterpret_cast<void *>(input1.data()),input1.size()*sizeof(int64_t),input_node_dims[1].data(),input_node_dims[1].size(),input_types[1],&input_tensors[1]));

    g_ort->ReleaseMemoryInfo(memory_info);

    CheckStatus(g_ort,g_ort->Run(session,nullptr,input_node_names.data(),(const OrtValue* const*)input_tensors.data(),
                                    input_tensors.size(), output_node_names.data(), output_node_names.size(), output_tensors.data()));

    return true;
}

bool ImageDetectionAI::get_detected_output_values(int* out_labels, float* out_boxes, float* out_scores, int* out_num_detected, int detect_limit=50){
    try{
        if (detect_limit>50 || detect_limit<0) detect_limit=50;
        if (detect_limit > _detect_limit) detect_limit = _detect_limit;

        memset(out_labels,0,50);
        memset(out_boxes,0,50*4);
        memset(out_scores,0,50);

        void* output_buffer0;
        void* output_buffer1;
        void* output_buffer2;
        size_t num_boxes = output_node_dims[2][1];
        CheckStatus(g_ort,g_ort->GetTensorMutableData(output_tensors[0],&output_buffer0));
        CheckStatus(g_ort,g_ort->GetTensorMutableData(output_tensors[1],&output_buffer1));
        CheckStatus(g_ort,g_ort->GetTensorMutableData(output_tensors[2],&output_buffer2));
        
        int64_t* labels_ptr = reinterpret_cast<int64_t*>(output_buffer0);
        float* boxes_ptr = reinterpret_cast<float*>(output_buffer1);
        float* scores_ptr = reinterpret_cast<float*>(output_buffer2);
        
        if (num_boxes > detect_limit) num_boxes=detect_limit;
        for (size_t i = 0; i < num_boxes; ++i)
        {
            *(out_labels+i) = *(labels_ptr+i);
        }
        memcpy(out_boxes,boxes_ptr,num_boxes*4);
        memcpy(out_scores,scores_ptr,num_boxes);
        *out_num_detected = num_boxes;

    }catch(std::exception &e){
        _last_error=makeLastError(std::format("exception occured - {}.",e.what()));
    }
    return true;
}

bool ImageDetectionAI::get_drawed_image_with_detected_output(unsigned char* out_image_data, int image_data_length, int num_detected, int* labels_ptr, float* boxes_ptr, float* scores_ptr){
    try{

        if (image_data_length != model_h*model_w*model_c){
            _last_error = makeLastError(std::format("image_data_length value({}) is not match with model's h*w*c ({}), model_h({}), model_w({}), model_c({})", image_data_length, model_h*model_w*model_c, model_h, model_w, model_c));
            return false;
        }
        
        // int* labels = new int[num_detected];
        // float* boxes = new float[num_detected*4];
        // float* scores = new float[num_detected];

        // cv::Mat(out_image_data,image_data_length);
        cv::Mat detected_image(model_h,model_w,CV_8UC3, out_image_data);

        for (size_t i = 0; i < num_detected; ++i)
        {
            if (*(scores_ptr+i)>score_thres){
                int x1 = (int)*(boxes_ptr+4*i);
                int y1 = (int)*(boxes_ptr+4*i+1);
                int x2 = (int)*(boxes_ptr+4*i+2);
                int y2 = (int)*(boxes_ptr+4*i+3);
                std::string temp_score = std::to_string(*(scores_ptr+i));
                size_t dot = temp_score.find(".");
                cv::putText(detected_image,
                            label_mapping[*(labels_ptr+i)] + " : "+ temp_score.replace(temp_score.begin()+dot+3,temp_score.end(),""),
                            cv::Point(x1,y1+25),
                            cv::FONT_HERSHEY_SIMPLEX,
                            1,
                            cv::Scalar(0,0,255)
                            );
                cv::rectangle(detected_image,cv::Rect(x1,y1,x2-x1,y2-y1),cv::Scalar(0,0,255));
            }
        }
    }catch(std::exception &e){
        _last_error=makeLastError(std::format("exception occured - {}.",e.what()));
    }
    return true;
}

bool ImageDetectionAI::get_detected_image(unsigned char* out_image_data, int image_data_length){
    try{

        if (image_data_length != model_h*model_w*model_c){
            _last_error = makeLastError(std::format("image_data_length value({}) is not match with model's h*w*c ({}), model_h({}), model_w({}), model_c({})", image_data_length, model_h*model_w*model_c, model_h, model_w, model_c));
            return false;
        }
        int detect_limit = _detect_limit;
        int* out_labels = new int[detect_limit];
        float* out_boxes = new float[detect_limit*4];
        float* out_scores = new float[detect_limit];
        int* out_num_detected= new int;

        if (!get_detected_output_values(out_labels, out_boxes, out_scores, out_num_detected, detect_limit)){
            _last_error = makeLastError("get_detected_output_values method failed.");
            return false;
        }

        if (!get_drawed_image_with_detected_output(out_image_data,image_data_length,*out_num_detected,out_labels,out_boxes,out_scores)){
            // last_error will be set in the function.
            return false;
        }

    
    }catch(std::exception &e){
        _last_error=makeLastError(std::format("exception occured - {}.",e.what()));
    }
    return true;
}



int main(){
    bool use_xnn=false;
    // bool use_xnn=false;
    // bool use_dml=true;
    bool use_dml=false;
    
    // bool use_dml=false;
    int intra_op_num_threads = 4;
    int infer_time_check_loop = 5;
    // std::string file_name = "11_0_test.png";
    std::string file_name = "the-lucky-neko-rplhB9mYF48-unsplash.jpg";
    std::wstring model_path = L"my_modified_rtdetector.onnx";
    int model_w = 640;
    int model_h = 640;
    float score_thres = 0.5;

    std::cout << "intra_op_num_threads : " << intra_op_num_threads << std::endl;
    std::cout << "loaded file_name : " << file_name << std::endl;
    if (use_xnn) std::cout << "use_xnn" << std::endl; 
    if (use_dml) std::cout << "use_dml" << std::endl;

    // std::string replace_string = "_resized.jpg";
    // const char* file_name{"11_0_test.png"};
    int image_w_orig, image_h_orig, image_comp_orig;
    cv::Mat loaded_image = cv::imread(file_name,1);
    image_w_orig = loaded_image.cols;
    image_h_orig = loaded_image.rows;
    image_comp_orig = loaded_image.dims;
    unsigned char* image_data = loaded_image.data;
    // unsigned char* image_data = stbi_load(file_name.c_str(),&image_w_orig,&image_h_orig,&image_comp_orig,0);

    int image_w_resized=model_w;
    int image_h_resized=model_h;
    // unsigned char* image_data_resized = stbir_resize_uint8_linear(image_data, image_w_orig, image_h_orig, image_w_orig*image_comp_orig,nullptr,image_w_resized,image_h_resized,image_w_resized*image_comp_orig,STBIR_RGB);
    // stbi_write_jpg(file_name.replace(file_name.end()-4,file_name.end(),"_resized.jpg").c_str(),image_w_resized,image_h_resized,image_comp_orig,image_data_resized,80);
    cv::Mat resized_image;
    cv::resize(loaded_image,resized_image,cv::Size(model_w,model_h),0,0,cv::INTER_LANCZOS4);
    unsigned char* image_data_resized = resized_image.data;

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
    size_t input_data_size = 1*model_h*model_w*3;
    const size_t num_elements = 1*model_h*model_w*3;

    CheckStatus(g_ort, g_ort->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info));
    size_t input_data_length = input_data_size * sizeof(uint8_t);
    CheckStatus(g_ort,g_ort->CreateTensorWithDataAsOrtValue(memory_info, reinterpret_cast<void*>(image_data_resized), input_data_length,
                            input_node_dims[0].data(), input_node_dims[0].size(), input_types[0], &input_tensors[0]));
    std::vector<int64_t> input1{model_h,model_w};
    CheckStatus(g_ort,g_ort->CreateTensorWithDataAsOrtValue(memory_info,reinterpret_cast<void *>(input1.data()),input1.size()*sizeof(int64_t),input_node_dims[1].data(),input_node_dims[1].size(),input_types[1],&input_tensors[1]));

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

// without nms
// no xnnpack, no dml prepare = 287 msec, sesseion_run ave : 165 msec
// true xnnpack, no dml prepare = 330 msec, sesseion_run ave : 235 msec
// no xnnpack, no dml prepare = 1000 msec, sesseion_run ave : 83 msec


#ifdef MyTimeCheck
    _MEASURE_END(onnxruntime_session_run,loop_count)
    // _MEASURE_END(onnxruntime_session_run,1)
#endif

    // float *output_data = (float*)malloc(1*3*512*512*sizeof(float));
    void* output_buffer0;
    void* output_buffer1;
    void* output_buffer2;
    size_t num_boxes = output_node_dims[2][1];
    CheckStatus(g_ort,g_ort->GetTensorMutableData(output_tensors[0],&output_buffer0));
    CheckStatus(g_ort,g_ort->GetTensorMutableData(output_tensors[1],&output_buffer1));
    CheckStatus(g_ort,g_ort->GetTensorMutableData(output_tensors[2],&output_buffer2));
    

    int64_t* labels_ptr = reinterpret_cast<int64_t*>(output_buffer0);
    float* boxes_ptr = reinterpret_cast<float*>(output_buffer1);
    float* scores_ptr = reinterpret_cast<float*>(output_buffer2);
    
    for (size_t i = 0; i < num_boxes; ++i)
    {
        if (*(scores_ptr+i)>score_thres){
            int x1 = (int)*(boxes_ptr+4*i);
            int y1 = (int)*(boxes_ptr+4*i+1);
            int x2 = (int)*(boxes_ptr+4*i+2);
            int y2 = (int)*(boxes_ptr+4*i+3);
            std::string temp_score = std::to_string(*(scores_ptr+i));
            size_t dot = temp_score.find(".");
            cv::putText(resized_image,
                        label_mapping[*(labels_ptr+i)] + " : "+ temp_score.replace(temp_score.begin()+dot+3,temp_score.end(),""),
                        cv::Point(x1,y1+25),
                        cv::FONT_HERSHEY_SIMPLEX,
                        1,
                        cv::Scalar(0,0,255)
                        );
            cv::rectangle(resized_image,cv::Rect(x1,y1,x2-x1,y2-y1),cv::Scalar(0,0,255));

        }
    }
    std::string temp = file_name.replace(file_name.end()-4,file_name.end(),"_output_test.jpg");
    
    cv::String cv_str_temp = cv::String(temp);
    // cv::imwrite(file_name.replace(file_name.end()-4,file_name.end(),"_output.jpg"),resized_image);
    cv::imwrite(cv_str_temp,resized_image);
    // cv::ImwriteFlags;

    // stbi_write_jpg(file_name.replace(file_name.end()-4,file_name.end(),"_output.jpg").c_str(),image_w_resized,image_h_resized,image_comp_orig,output_buffer,80);

    return 0;
}
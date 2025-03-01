#include "onnxruntime_cxx_api.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#if defined(_WIN32)
#define EXPORT_PREFIX __declspec(dllexport)
#else
#define EXPORT_PREFIX __attribute__((visibility("default")))
#endif

enum EXECUTION_PROVIDER{
    CPU,
    GPU,
    NPU,
    DML,
    XNNPACK
};


class ImageDetectionAI{
    public:
    ImageDetectionAI();
    int model_h;
    int model_w;
    int model_c;
    

    int intra_op_num_threads;
    int infer_time_check_loop;
    float score_thres = 0.5;
    // std::string file_name = "the-lucky-neko-rplhB9mYF48-unsplash.jpg";
    // std::wstring model_path = L"my_modified_rtdetector.onnx";
    
    cv::Mat resizedImage;

    bool load_model(const char* model_path);
    bool set_execution_provider(const char* provider);
    bool inference_from_file(const char* file_path);
    bool inference_from_bytes(unsigned char* buffer_data, int length);
    bool get_detected_output_values(int* out_labels, float* out_boxes, float* out_scores, int* out_num_detected, int detect_limit);
    bool get_drawn_image_with_detected_output(unsigned char* out_image, int image_data_length, int num_detected, int* labels, float* boxes, float* scores);
    bool get_detected_image(unsigned char* out_image, int image_data_length);
    const char* get_last_error();
    bool is_model_loaded();
    
    private:

    std::string _model_path;
    bool _is_model_loaded=false;
    int _execution_provider;
    int _detect_limit=50;
    std::string _last_error="";
    const OrtApi* g_ort;
    OrtEnv* env;
    OrtSessionOptions* session_options;

    OrtSession* session;
    OrtAllocator* allocator;
    size_t num_input_nodes;

    std::vector<const char*> input_node_names;
    std::vector<std::vector<int64_t>> input_node_dims;
    std::vector<ONNXTensorElementDataType> input_types;
    std::vector<OrtValue*> input_tensors;
    
    size_t num_output_nodes;
    std::vector<const char*> output_node_names;
    std::vector<std::vector<int64_t>> output_node_dims;
    std::vector<OrtValue*> output_tensors;
};

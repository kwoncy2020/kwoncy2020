import 'dart:ffi' as ffi;
import 'dart:io' show Platform, Directory;
import 'dart:io';
import 'package:ffi/ffi.dart';
import 'package:path/path.dart' as path;
import 'package:llama_cpp_dart/llama_cpp_dart.dart';

// void tflite_c(unsigned char* buffer_func_input, const char* model_name_input,int byteSize_input, int num_threads_input=4, int model_bits_input=8, bool is_model_qat_input=false, bool use_xnn_input=false, bool use_gpu_input=false, bool use_npu_input=false, int loop_count_time_input=1)

typedef NativeFuncImageEnhance = ffi.Void Function(
    ffi.Pointer<ffi.UnsignedChar> bufferN,
    ffi.Pointer<Utf8> nameN,
    ffi.Int32 byteSizeN,
    ffi.Int32 numThreadsN,
    ffi.Int32 numBitsN,
    ffi.Bool isModelQatN,
    ffi.Bool useXnnN,
    ffi.Bool useGpuN,
    ffi.Bool useNpuN,
    ffi.Int32 timeCheckLoopN);
typedef DartFuncImageEnhance = void Function(
    ffi.Pointer<ffi.UnsignedChar> buffer,
    ffi.Pointer<Utf8> name,
    int byteSize,
    int numThreads,
    int numBits,
    bool isModelQat,
    bool useXnn,
    bool useGpu,
    bool useNpu,
    int timeCheckLoop);

typedef NativeFuncChatAICreateChatAI = ffi.Pointer<ffi.Void> Function(
    ffi.Pointer<Utf8> model_path, ffi.Pointer<Utf8> execution_provider);
typedef DartFuncChatAICreateChatAI = ffi.Pointer<ffi.Void> Function(
    ffi.Pointer<Utf8> model_path, ffi.Pointer<Utf8> execution_provider);

typedef NativeFuncChatAIDeleteChatAI = ffi.Void Function(
    ffi.Pointer<ffi.Void> ptr);
typedef DartFuncChatAIDeleteChatAI = void Function(ffi.Pointer<ffi.Void>);

// generate whole sentence at once. (non asinc way, time consuming)
typedef NativeFuncChatAIGenerateTextFromChatAI = ffi.Pointer<Utf8> Function(
    ffi.Pointer<ffi.Void> ptr, ffi.Pointer<Utf8> prompt);
typedef DartFuncChatAIGenerateTextFromChatAI = ffi.Pointer<Utf8> Function(
    ffi.Pointer<ffi.Void> ptr, ffi.Pointer<Utf8> prompt);

typedef NativeFuncChatAISetPrompt = ffi.Bool Function(
    ffi.Pointer<ffi.Void> ptr, ffi.Pointer<Utf8> prompt);
typedef DartFuncChatAISetPrompt = bool Function(
    ffi.Pointer<ffi.Void> ptr, ffi.Pointer<Utf8> prompt);

typedef NativeFuncChatAIIsDone = ffi.Bool Function(ffi.Pointer<ffi.Void> ptr);
typedef DartFuncChatAIIsDone = bool Function(ffi.Pointer<ffi.Void> ptr);

typedef NativeFuncChatAIGenerateNext = ffi.Pointer<Utf8> Function(
    ffi.Pointer<ffi.Void> ptr);
typedef DartFuncChatAIGenerateNext = ffi.Pointer<Utf8> Function(
    ffi.Pointer<ffi.Void> ptr);

class AIuser {
  AIuser(
      {this.modelName = "",
      this.numBits =
          8, // Todo: currently 8bit for hardware acceleration image. This should be change.
      this.numThreads = 1,
      this.inferDevice = "cpu",
      this.isSharedLibLoaded = false});

  String modelName;
  int numBits;
  int numThreads;
  String inferDevice;
  bool isSharedLibLoaded;
}

class LowLightEnhanceAI extends AIuser {
  LowLightEnhanceAI(
      {super.modelName = "",
      super.numBits = 8,
      super.numThreads = 1,
      super.inferDevice = "cpu"});
  int w = 512;
  int h = 512;
  int c = 3;
  ffi.DynamicLibrary? dyLib;
  //
  // tflite_c(buffer, byteSize, model_name, num_threads, model_bits, is_model_qat);

  void loadLibrary() {
    // var dllPath = path.join(Directory.current.path,"lib","using_tfl_c_shared.dll");
    // var dllPath = path.join(Directory.current.path, "build", "native_assets",
    //     "windows", "tensorflowlite_c", "flutter_my_ai.dll");
    var dllPath = "flutter_my_ai.dll";

    dyLib = ffi.DynamicLibrary.open(dllPath);
    if (dyLib?.providesSymbol("tflite_c") == false) {
      print("dll load fail");
      isSharedLibLoaded = false;
      return;
    }
    isSharedLibLoaded = true;

    return;
  }

  void aiEnhanceImage(ffi.Pointer<ffi.UnsignedChar> imgBufferPtr) {
    if (!this.isSharedLibLoaded) {
      print("Dll not loaded");
      return;
    }
    var funcPointer =
        dyLib?.lookup<ffi.NativeFunction<NativeFuncImageEnhance>>("tflite_c");
    // // dyLib?.lookupFunction(symbolName)
    var dartF = funcPointer!.asFunction<DartFuncImageEnhance>();
    dartF(
        imgBufferPtr,
        "C:\\Users\\kwoncy\\Documents\\dev\\git_upload\\flutter_ai_app\\flutter_using_dll\\flutter_using_dll\\build\\native_assets\\windows\\zdce++enhanced_out_okv3_f_int_q_uint8.tflite"
            .toNativeUtf8(),
        /* byteSize */ 512 * 512 * 3,
        /* num_threads */ 4,
        /* model_bits */ 8,
        /* is_QAT */ false,
        /* use_xnn */ true,
        /* use_gpu */ false,
        /* use_npu */ false,
        /* time_check_loop */ 0);
  }
}

class ChatAI extends AIuser {
  ChatAI(
      {super.modelName = "",
      super.numBits = 32,
      super.numThreads = 1,
      super.inferDevice = "cpu"});

  late Llama model;
  bool isModelLoaded = false;
  String _lastError = "";

  String get getLastError {
    return _lastError;
  }

  void loadModel(String libraryPath, String modelPath) {
    try {
      // Llama.libraryPath = libraryPath;
      Llama.libraryPath = "llama.dll";
      // Llama.libraryPath =
      // "C:\\Users\\kwoncy\\Documents\\dev\\mygit\\practices\\flutter_ai_app1\\build\\native_assets\\windows\\llama\\llama.dll";
      // "C:/Users/kwoncy/Documents/dev/mygit/practices/flutter_ai_app1/build/native_assets/windows/llama/llama.dll";
      model = Llama(modelPath);

      // Todo : set modelName, numBits, numThreads, inferDevice

      // final loadCommand = LlamaLoad(
      // path: "path/to/model.gguf",
      // modelParams: ModelParams(),
      // contextParams: ContextParams(),
      // samplingParams: SamplerParams(),
      // format: ChatMLFormat(),
      // );

      // final llamaParent = LlamaParent(loadCommand);
      // llamaParent.init();

      // llamaParent.stream.listen((response) => print(response));
      // llamaParent.sendPrompt("2 * 2 = ?");

      isSharedLibLoaded = true;
      isModelLoaded = true;
    } catch (e) {
      print(e);
      _lastError = "$e";
    }
  }

  String generateWholeTokens(String prompt) {
    if (!isModelLoaded) return "the model is not loaded";
    String ret = "";
    model.setPrompt(prompt);
    while (true) {
      var (token, done) = model.getNext();
      ret += token;
      if (done) break;
    }
    return ret;
  }

  Stream<String> generateNextToken(String prompt) async* {
    model.setPrompt(prompt);
    while (true) {
      var (token, done) = model.getNext();
      if (done) break;
      yield token;
    }
  }
}

class ChatAIORT extends AIuser {
  ChatAIORT(
      {super.modelName = "",
      super.numBits = 32, // Todo : set quantized bits from loaded model file.
      super.numThreads = 1, // Todo : set quantized bits from loaded model file.
      super.inferDevice = "cpu"});

  ffi.DynamicLibrary? dyLib;
  bool isModelLoaded = false;
  String _lastError = "";
  ffi.Pointer<ffi.Void> modelptr = ffi.nullptr;

  String get getLastError {
    return _lastError;
  }

  bool loadLibrary(String sharedLibPath) {
    try {
      if (sharedLibPath.isEmpty) {
        sharedLibPath = "my_chat_ai";
        if (Platform.isWindows) sharedLibPath += ".dll";
        if (Platform.isAndroid) sharedLibPath += ".so";
      }

      dyLib = ffi.DynamicLibrary.open(sharedLibPath);
      if (dyLib?.providesSymbol("create_chat_ai") == false) {
        _lastError = "shared lib load fail";
        isSharedLibLoaded = false;
        return false;
      }

      isSharedLibLoaded = true;
      isModelLoaded = true;
      return true;
    } catch (e) {
      print(e);
      _lastError = "$e";
    }
    return false;
  }

  bool loadModel(String modelPath, String executionProvider) {
    if (!isSharedLibLoaded) {
      _lastError =
          "from ChatAIORT::loadModel : load fail - shared library hasn't been loaded.";
      return false;
    }
    ;
    modelptr = create_chat_ai(
        modelPath.toNativeUtf8(), executionProvider.toNativeUtf8());
    if (modelptr == ffi.nullptr) {
      isModelLoaded = false;
      _lastError =
          "from ChatAIORT::loadModel : load fail (received nullptr from native create_chat_ai function)";
      return false;
    } else {
      isModelLoaded = true;
      return true;
    }
  }

  ffi.Pointer<ffi.Void> create_chat_ai(
      ffi.Pointer<Utf8> model_path, ffi.Pointer<Utf8> execution_provider) {
    if (!isSharedLibLoaded) {
      _lastError = "from ChatAIORT::create_chat_ai : shared lib not loaded";
      return ffi.nullptr;
    }

    var funcPointer = dyLib!
        .lookup<ffi.NativeFunction<NativeFuncChatAICreateChatAI>>(
            "create_chat_ai");
    var dartF = funcPointer.asFunction<DartFuncChatAICreateChatAI>();

    return dartF(model_path, execution_provider);
  }

  void delete_chat_ai(ffi.Pointer<ffi.Void> ptr) {
    var funcPointer = dyLib!
        .lookup<ffi.NativeFunction<NativeFuncChatAIDeleteChatAI>>(
            "delete_chat_ai");
    var dartF = funcPointer.asFunction<DartFuncChatAIDeleteChatAI>();
    dartF(ptr);
  }

  // This method will return whole generated text. not async and time consuming.
  ffi.Pointer<Utf8> generate_text_from_chat_ai(
      ffi.Pointer<ffi.Void> ptr, ffi.Pointer<Utf8> prompt) {
    var funcPointer = dyLib!
        .lookup<ffi.NativeFunction<NativeFuncChatAIGenerateTextFromChatAI>>(
            "generate_text_from_chat_ai");

    var dartF = funcPointer.asFunction<DartFuncChatAIGenerateTextFromChatAI>();
    return dartF(ptr, prompt);
  }

  // Will return false if the generator not finished it's previous task. similar generator.is_done()
  bool set_prompt(ffi.Pointer<ffi.Void> ptr, ffi.Pointer<Utf8> prompt) {
    var funcPointer = dyLib!
        .lookup<ffi.NativeFunction<NativeFuncChatAISetPrompt>>("set_prompt");
    var dartF = funcPointer.asFunction<DartFuncChatAISetPrompt>();
    return dartF(ptr, prompt);
  }

  bool is_done(ffi.Pointer<ffi.Void> ptr) {
    var funcPointer =
        dyLib!.lookup<ffi.NativeFunction<NativeFuncChatAIIsDone>>("is_done");
    var dartF = funcPointer.asFunction<DartFuncChatAIIsDone>();
    return dartF(ptr);
  }

  ffi.Pointer<Utf8> generate_next(ffi.Pointer<ffi.Void> ptr) {
    var funcPointer = dyLib!
        .lookup<ffi.NativeFunction<NativeFuncChatAIGenerateNext>>(
            "generate_next");
    var dartF = funcPointer.asFunction<DartFuncChatAIGenerateNext>();
    return dartF(ptr);
  }

  String generateWholeText(String prompt) {
    if (!isModelLoaded) {
      _lastError =
          "from ChatAIORT::generateWholeText : the model is not loaded.";
      return "from ChatAIORT::generateWholeText : the model is not loaded.";
    }

    return generate_text_from_chat_ai(modelptr, prompt.toNativeUtf8())
        .toDartString();
  }

  Stream<String> generateNextText(String prompt) async* {
    if (!isModelLoaded) {
      _lastError =
          "from ChatAIORT::generateNextText : the model is not loaded.";
      yield "from ChatAIORT::generateNextText : the model is not loaded.";
    } else {
      if (prompt.isEmpty) {
        yield "input your question.";
      } else {
        bool res = set_prompt(modelptr, prompt.toNativeUtf8());
        if (!res) {
          _lastError =
              "from ChatAIORT::generateNextText : set_prompt method failed.";
          yield "from ChatAIORT::generateNextText : set_prompt method failed.";
        } else {
          while (!is_done(modelptr)) {
            await Future.delayed(const Duration(milliseconds: 50));
            ffi.Pointer<Utf8> c_text = generate_next(modelptr);
            String text = c_text.toDartString();
            yield text;
          }
        }
      }
    }
  }
}

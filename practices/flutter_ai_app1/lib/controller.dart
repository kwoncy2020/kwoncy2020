import 'dart:async';
import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'dart:io' show Platform, Directory;

import 'package:flutter/material.dart';
import 'package:get/get.dart';
import 'model.dart';
import 'package:path/path.dart' as path;
import 'dart:typed_data';
import 'package:file_picker/file_picker.dart';
import 'package:image/image.dart' as img;

class ChatAIController extends GetxController {
  var chatAI = ChatAI();
  var chatText = Rx<List<RxString>>([]);
  RxString generateText = RxString("");

  var dllPath = path.join(Directory.current.path, "build", "native_assets",
      "windows", "llama", "llama.dll");
  var modelPath = path.join(Directory.current.path, "build", "flutter_assets",
      "DeepSeek-R1-Distill-Qwen-1.5B-Q5_K_M.gguf");
  // var modelPath = "DeepSeek-R1-Distill-Qwen-1.5B-Q5_K_M.gguf";

  @override
  @mustCallSuper
  void onInit() {
    super.onInit();
    chatAI.loadModel(dllPath, modelPath);
  }

  void generate(String prompt) {
    if (!chatAI.isModelLoaded) {
      // generateText.value = "the model is not loaded";
      // chatText.value.add(RxString("the model is not loaded"));
      chatText.refresh();

      return;
    }
    chatText.value.add(RxString(prompt));
    chatText.value.add(generateText);
    chatText.refresh();
    // chatText.update();

    chatAI.generateNextToken(prompt).forEach((str) {
      generateText.value += str;
      generateText.refresh();
      chatText.refresh();
    });
    // generateText.refresh();
    generateText.refresh();
    // chatText.refresh();
    // chatText.update();
  }

  void generateWhole(String prompt) {
    if (!chatAI.isModelLoaded) {
      // generateText.value = "the model is not loaded";
      chatText.value.add(RxString("the model is not loaded"));
      chatText.refresh();
      return;
    }

    chatText.value.add(RxString(prompt));
    chatText.refresh();
    // generateText.value = chatAI.generateWholeTokens(prompt);
    chatText.value.add(RxString(chatAI.generateWholeTokens(prompt)));
    chatText.refresh();
  }
}

class ChatAIORTController extends GetxController {
  var chatAIORT = ChatAIORT();
  // var chatTextHistory = Rx<List<RxString>>([]);
  var chatTextHistory = Rx<List<String>>(
      ["welcome to my chat ai!", "currently this model uses phi3.5-mini"]);
  String generatedText = "";
  RxString rxGeneratedText = "".obs;
  StreamController<String> textStreamController =
      StreamController<String>.broadcast();

  bool isPrevGenDone = true;
  String prompt_ = "";
  // rxGeneratedText
  // rxString.bin
  // var sharedLibName = "my_chat_ai";
  String sharedLibName = "my_chat_ai";
  String libExtension = Platform.isWindows ? ".dll" : ".so";
  // if (Platform.isWindows) sharedLibName += ".dll";

  // var sharedLibPath = path.join(Directory.current.path, "build",
  //     "native_assets", "windows", "llama", ".dll");
  late var sharedLibPath = sharedLibName + libExtension;
  var modelPath = path.join(Directory.current.path, "build", "flutter_assets",
      "cpu-int4-awq-block-128-acc-level-4");

  @override
  @mustCallSuper
  void onInit() {
    super.onInit();
    if (chatAIORT.loadLibrary(sharedLibPath)) {
      chatAIORT.loadModel(modelPath, "cpu");
    }
    textStreamController.stream.listen((e) {
      generatedText += e;
    });
  }

  void clearGeneratedText() {
    if (generatedText.isNotEmpty) generatedText = "";
  }

  void clearInnerPrompt() {
    if (prompt_.isNotEmpty) prompt_ = "";
  }

  void addPromptToHistory(String prompt) {
    chatTextHistory.value.add(prompt);
    chatTextHistory.refresh();
    update();

    return;
  }

  bool answerPrompt(String prompt) {
    if (!isPrevGenDone) {
      return false;
    }
    addPromptToHistory(prompt);
    clearInnerPrompt();
    clearGeneratedText();
    prompt_ = prompt;
    var stream = chatAIORT.generateNextText(prompt_).asBroadcastStream();
    textStreamController.addStream(stream);
    // textStreamController.
    // textStreamController.stream.listen((e) {
    //   generatedText += e;
    // });

    // rxGeneratedText.bindStream(generate(prompt));
    return true;
  }

  Stream<String> generate() async* {
    if (!chatAIORT.isModelLoaded) {
      return;
    }

    if (generatedText.isNotEmpty) {
      generatedText = "";
    }
    if (prompt_.isEmpty) {
      yield "you can enter your question.";
      return;
    }
    if (!isPrevGenDone) {
      yield "generator is busy.";
      return;
    }
    isPrevGenDone = false;
    // RxString prompt_ = RxString(prompt);
    // // chatTextHistory.value.add(prompt_);
    // chatTextHistory.update((v) => v!.add(prompt_));
    // // RxString generatedText = RxString("");
    // // chatTextHistory.value.add(generatedText);

    // // chatTextHistory.refresh();
    // var newStream = chatAIORT.generateNextText(prompt_);
    // while (newStream.)
    // yield* chatAIORT.generateNextText(prompt_).asyncExpand((s) {
    //   generatedText += s;
    //   yield * generatedText;
    // });
    // chatAIORT.generateNextText(prompt);
    // textStreamController.addStream(chatAIORT.generateNextText(prompt_));
    // yield* textStreamController.stream.;
    // addPromptToHistory(generatedText);
    isPrevGenDone = true;
    // generatedText.refresh();
    // chatTextHistory.refresh();
  }

//   void generateWhole(String prompt) {
//     if (!chatAIORT.isModelLoaded) {
//       chatTextHistory.value.add(RxString("the model is not loaded"));
//       chatTextHistory.refresh();
//       return;
//     }

//     chatTextHistory.value.add(RxString(prompt));
//     chatTextHistory.refresh();
//     chatTextHistory.value.add(RxString(chatAIORT.generateWholeText(prompt)));
//     chatTextHistory.refresh();
//   }
}

class ImageEnlightAIController extends GetxController {
  var imageEnlightAI = LowLightEnhanceAI(
      modelName: "image_enhanced_out_okv3_f_int_q_uint8.tflite",
      inferDevice: "cpu",
      numBits: 8,
      numThreads: 4);
  String sharedLibPath = "";
  String modelPath = path.join(Directory.current.path, "build",
      "flutter_assets", "image_enhanced_out_okv3_f_int_q_uint8.tflite");

  RxBool sharedLibLoadCheck = false.obs;
  var imageBytes = Uint8List(0).obs;

  var filePickerController = FilePickerContorller();

  @override
  void onInit() {
    super.onInit();
    sharedLibPath = "flutter_my_ai";
    if (Platform.isWindows) sharedLibPath += ".dll";
    if (Platform.isAndroid) sharedLibPath += ".so";

    imageEnlightAI.loadLibrary(sharedLibPath);
    imageEnlightAI.setModelPath(modelPath);
    if (imageEnlightAI.isSharedLibLoaded) {
      sharedLibLoadCheck.value = true;
      sharedLibLoadCheck.refresh();
    }
  }

  bool loadAILibrary() {
    if (imageEnlightAI.isSharedLibLoaded) {
      sharedLibLoadCheck.value = true;
      sharedLibLoadCheck.refresh();
      return true;
    } else {
      imageEnlightAI.loadLibrary(sharedLibPath);
      sharedLibLoadCheck.value = imageEnlightAI.isSharedLibLoaded;
      sharedLibLoadCheck.refresh();
      return imageEnlightAI.isSharedLibLoaded;
    }
  }

  Future<bool?> loadImageFromFile() async {
    var files = await filePickerController.selectFiles();

    if (files == null) {
      // imageBytes.value.clear();
      return false;
    } else {
      if (files != null && files.first.bytes != null) {
        imageBytes.value = files.first.bytes!;
        imageBytes.refresh();
        // print("imageBytes set.");
        return true;
      }
    }
    return null;
  }

  void imageEnlight() {
    if (imageBytes.value.isEmpty) return;

    img.Image? tempImg = img.decodeImage(imageBytes.value);
    // img.Image? tempImg = img.decodeJpg(imageBytes.value);
    // img.Image? tempImg = img.Image.fromBytes(width: width, height: height, bytes: bytes);
    if (tempImg == null) return;

    int origImgWidth = tempImg.width;
    int origImgHeight = tempImg.height;

    img.Image resizedTempImg = img.copyResize(tempImg,
        width: imageEnlightAI.w,
        height: imageEnlightAI.h,
        interpolation: img.Interpolation.cubic);

    int tempMemorySize = imageEnlightAI.w *
        imageEnlightAI.h *
        imageEnlightAI.c *
        sizeOf<UnsignedChar>(); // 786432
    // var byteData = resizedTempImg.data!.buffer.asByteData();
    Pointer<UnsignedChar> tempMemory =
        malloc.allocate<UnsignedChar>(tempMemorySize);
    tempMemory
        .cast<Uint8>()
        .asTypedList(tempMemorySize)
        .setAll(0, resizedTempImg.buffer.asUint8List());

    bool enlightResult = imageEnlightAI.aiEnhanceImage(tempMemory);
    if (!enlightResult) {
      debugPrint(imageEnlightAI.lastError);
      return;
    }
    var tempImage = img.Image.fromBytes(
        width: imageEnlightAI.w,
        height: imageEnlightAI.h,
        bytes: tempMemory
            .cast<Uint8>()
            .asTypedList(imageEnlightAI.w * imageEnlightAI.h * imageEnlightAI.c)
            .buffer);

    img.Image resizedToOriginalImage = img.copyResize(tempImage,
        width: origImgWidth,
        height: origImgHeight,
        interpolation: img.Interpolation.cubic);
    // imageBytes.value = tempMemory.cast<Uint8>().asTypedList(tempMemorySize);
    // imageBytes.value = resizedTempImg.buffer.asUint8List();
    // imageBytes.value = img.JpegEncoder(quality: 100).encode(resizedTempImg);
    // imageFileName.contains(".jpg")

    imageBytes.value = img.JpegEncoder(quality: 100).encode(img.copyCrop(
        resizedToOriginalImage,
        x: 0,
        y: 0,
        width: origImgWidth,
        height: origImgHeight));

    malloc.free(tempMemory);
  }
}

class ImageDetectionAIController extends GetxController {
  var imageDetectionAI = ImageDetectionAI(
      modelName: "phi-3.5-mini",
      numBits: 32,
      numThreads: 4,
      inferDevice: "cpu");

  String sharedLibPath = "";
  String modelPath = path.join(Directory.current.path, "build",
      "flutter_assets", "my_modified_rtdetector.onnx");

  RxBool sharedLibLoadCheck = false.obs;
  late int detect_limit = imageDetectionAI.detect_limit;
  var imageBytes = Uint8List(0).obs;
  var out_num_detected = Int32().obs;
  late var out_labels = Int32List(detect_limit).obs;
  late var out_boxes = Float32List(detect_limit).obs;
  late var out_scores = Float32List(detect_limit).obs;

  var filePickerController = FilePickerContorller();

  @override
  void onInit() {
    super.onInit();
    sharedLibPath = "my_detection_ai";
    if (Platform.isWindows) sharedLibPath += ".dll";
    if (Platform.isAndroid) sharedLibPath += ".so";

    imageDetectionAI.loadLibrary(sharedLibPath);
    var provider = imageDetectionAI.inferDevice;
    if (provider == "cpu") {
      provider = "xnnpack";
    }
    imageDetectionAI.loadModel(modelPath, provider);

    if (imageDetectionAI.isSharedLibLoaded) {
      sharedLibLoadCheck.value = true;
      sharedLibLoadCheck.refresh();
    }
  }

  Future<bool?> loadImageFromFile() async {
    var files = await filePickerController.selectFiles();

    if (files == null) {
      // imageBytes.value.clear();
      return false;
    } else {
      if (files != null && files.first.bytes != null) {
        imageBytes.value = files.first.bytes!;
        imageBytes.refresh();
        // print("imageBytes set.");
        return true;
      }
    }
    return null;
  }

  // bool get
}

class FilePickerContorller extends GetxController {
  var selectedFirstFile = "";

  Future<List<PlatformFile>?> selectFiles() async {
    var result = await FilePicker.platform.pickFiles(
      type: FileType.image,
      allowedExtensions: ['jpg', 'png'],
      withData: true,
    );

    if (result != null && result.files.isNotEmpty) {
      selectedFirstFile = result.files.first.name;
      debugPrint(selectedFirstFile);
    }
    List<PlatformFile>? files = result?.files;
    return files;
  }
}

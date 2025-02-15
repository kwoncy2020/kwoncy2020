import 'dart:async';

import 'package:flutter/material.dart';
import 'package:get/get.dart';
import 'model.dart';
import 'dart:io' show Platform, Directory;
import 'package:path/path.dart' as path;

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
  var imageEnlightAI = LowLightEnhanceAI();
}

class FilePickerContorller extends GetxController {}

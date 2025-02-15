import 'dart:async';
import 'dart:ffi';
import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_ai_app1/controller.dart';
import 'model.dart';
import 'package:get/get.dart';
import 'package:file_picker/file_picker.dart';
// import 'dart:async';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';
import 'views/chat_page.dart';

void main() {
  runApp(MyApp());
  // runApp(StreamTestWithoutGetx());
}

//  Todo : Seperate remained code to model and controller.
class MyApp extends StatelessWidget {
  MyApp({super.key});
  var imageEnlightAIController = Get.put(ImageEnlightAIController());
  String imageFileName = "";

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return GetMaterialApp(
      home: Scaffold(
        body: Obx(
          () => Center(
            child: Column(
              children: [
                ElevatedButton(
                  // onPressed: () => Get.to(() => MyChatView()),
                  onPressed: () => Get.to(() => MyChatViewSTF()),
                  child: const Text("chatAI"),
                ),
                Text(
                  imageEnlightAIController.sharedLibLoadCheck.value == true
                      ? "loaded"
                      : "not loaded",
                ),
                ElevatedButton(
                  onPressed: () {
                    imageEnlightAIController.loadAILibrary();
                  },
                  child: const Text(
                    "load dll",
                  ),
                ),
                Row(mainAxisAlignment: MainAxisAlignment.center, children: [
                  ElevatedButton(
                    onPressed: () {
                      imageEnlightAIController.loadImageFromFile();
                    },
                    child: const Text(
                      "load image",
                    ),
                  ),
                  ElevatedButton(
                      onPressed: () {
                        imageEnlightAIController.imageEnlight();
                      },
                      child: const Text("enhance light"))
                ]),
                if (imageEnlightAIController.imageBytes.value.isNotEmpty)
                  Expanded(
                    child: SingleChildScrollView(
                      child: Container(
                          child: Image.memory(
                        imageEnlightAIController.imageBytes.value,
                      )),
                    ),
                  ),

                // imageBytes.isNotEmpty && imageBytes.length > 0? Image.memory(ikmageScreenController.bytes.value) : Container()
              ],
            ),
          ),
        ),
      ),
    );
  }
}

Stream<int> intStream() async* {
  int out = 0;
  while (true) {
    // sleep(Duration(seconds: 1));
    await Future.delayed(Duration(seconds: 1));
    out += 1;
    yield out;
  }
}

class StreamTestWithoutGetx extends StatefulWidget {
  const StreamTestWithoutGetx({super.key});

  @override
  State<StreamTestWithoutGetx> createState() => _StreamTestWithoutGetxState();
}

class _StreamTestWithoutGetxState extends State<StreamTestWithoutGetx> {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        body: Column(
          children: [
            StreamBuilder(
              stream: Stream.periodic(Duration(seconds: 1), (int count) {
                return count;
              }),
              builder: (context, snapshot) =>
                  Text("snapshot data : ${snapshot.data}"),
            ),
            StreamBuilder(
              stream: intStream(),
              builder: (context, snapshot) =>
                  Text("snapshot data : ${snapshot.data}"),
            ),
          ],
        ),
      ),
    );
  }
}

import 'dart:async';
import 'dart:ffi';
import 'dart:io';

import 'package:flutter/material.dart';
import 'model.dart';
import 'package:get/get.dart';
import 'package:file_picker/file_picker.dart';
// import 'dart:async';
import 'dart:typed_data';
import 'package:image/image.dart' as img;
import 'package:ffi/ffi.dart';
import 'views/chat_page.dart';

void main() {
  runApp(MyApp());
  // runApp(StreamTestWithoutGetx());
}

//  Todo : Seperate remained code to model and controller.
class MyApp extends StatelessWidget {
  MyApp({super.key});
  var myAI = LowLightEnhanceAI(
      modelName: "image_enhanced_out_okv3_f_int_q_uint8.tflite",
      inferDevice: "cpu",
      numBits: 8,
      numThreads: 4);
  RxBool loadCheck = false.obs;
  var imageBytes = Uint8List(0).obs;
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
                  child: Text("chatAI"),
                ),
                Text(
                  loadCheck.value == true ? "loaded" : "not loaded",
                ),
                ElevatedButton(
                  onPressed: () {
                    myAI.loadLibrary();
                    if (myAI.dyLib?.providesSymbol("tflite_c") == true)
                      loadCheck.value = true;
                    else
                      loadCheck.value = false;
                  },
                  child: Text(
                    "load dll",
                  ),
                ),
                Row(mainAxisAlignment: MainAxisAlignment.center, children: [
                  ElevatedButton(
                    onPressed: () async {
                      var result = await FilePicker.platform.pickFiles(
                        type: FileType.image,
                        allowedExtensions: ['jpg', 'png'],
                        withData: true,
                      );

                      if (result != null && result.files.isNotEmpty) {
                        imageFileName = result.files.first.name;
                        debugPrint(imageFileName);
                      }
                      List<PlatformFile>? files = result?.files;
                      if (files != null && files.first.bytes != null) {
                        imageBytes.value = files.first.bytes!;
                        print("imageBytes set.");
                      }
                    },
                    child: Text(
                      "load image",
                    ),
                  ),
                  ElevatedButton(
                      onPressed: () {
                        if (imageBytes.value.isEmpty) return;

                        img.Image? tempImg = img.decodeImage(imageBytes.value);
                        // img.Image? tempImg = img.decodeJpg(imageBytes.value);
                        // img.Image? tempImg = img.Image.fromBytes(width: width, height: height, bytes: bytes);

                        if (tempImg == null) return;
                        img.Image resizedTempImg = img.copyResize(tempImg,
                            width: 512,
                            height: 512,
                            interpolation: img.Interpolation.cubic);

                        int tempMemorySize =
                            512 * 512 * 3 * sizeOf<UnsignedChar>(); // 786432
                        // var byteData = resizedTempImg.data!.buffer.asByteData();
                        Pointer<UnsignedChar> tempMemory =
                            malloc.allocate<UnsignedChar>(tempMemorySize);
                        tempMemory
                            .cast<Uint8>()
                            .asTypedList(tempMemorySize)
                            .setAll(0, resizedTempImg.buffer.asUint8List());
                        myAI.aiEnhanceImage(tempMemory);
                        var tempImage = img.Image.fromBytes(
                            width: 512,
                            height: 512,
                            bytes: tempMemory
                                .cast<Uint8>()
                                .asTypedList(512 * 512 * 3)
                                .buffer);
                        // imageBytes.value = tempMemory.cast<Uint8>().asTypedList(tempMemorySize);
                        // imageBytes.value = resizedTempImg.buffer.asUint8List();
                        // imageBytes.value = img.JpegEncoder(quality: 100).encode(resizedTempImg);
                        // imageFileName.contains(".jpg")
                        imageBytes.value = img.JpegEncoder(quality: 100).encode(
                            img.copyCrop(tempImage,
                                x: 0, y: 0, width: 512, height: 512));
                        malloc.free(tempMemory);
                      },
                      child: Text("enhance light"))
                ]),
                if (imageBytes.value.isNotEmpty)
                  Expanded(
                    child: SingleChildScrollView(
                      child: Container(
                          child: Image.memory(
                        imageBytes.value,
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

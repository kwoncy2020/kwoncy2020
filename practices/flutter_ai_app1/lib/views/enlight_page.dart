import 'package:flutter/material.dart';
import 'package:flutter_ai_app1/controller.dart';
import 'package:get/get.dart';

class MyEnlightView extends StatelessWidget {
  MyEnlightView({super.key});
  var imageEnlightAIController = Get.put(ImageEnlightAIController());

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Obx(() => Center(
            child: Column(
              children: [
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
              ],
            ),
          )),
    );
  }
}

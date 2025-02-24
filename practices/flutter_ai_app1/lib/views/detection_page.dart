import 'package:flutter/material.dart';
import 'package:flutter_ai_app1/controller.dart';
import 'package:get/get.dart';

class MyDetectionView extends StatelessWidget {
  MyDetectionView({super.key});
  var imageDetectionAIController = Get.put(ImageDetectionAIController());

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Obx(() => Center(
            child: Column(
              children: [
                Row(mainAxisAlignment: MainAxisAlignment.center, children: [
                  ElevatedButton(
                    onPressed: () {
                      imageDetectionAIController.loadImageFromFile();
                    },
                    child: const Text(
                      "load image",
                    ),
                  ),
                  ElevatedButton(
                      onPressed: () {
                        imageDetectionAIController.getDetectedImage();
                      },
                      child: const Text("get detected image"))
                ]),
                if (imageDetectionAIController.imageBytes.value.isNotEmpty)
                  Expanded(
                    child: SingleChildScrollView(
                      child: Container(
                          child: Image.memory(
                        imageDetectionAIController.imageBytes.value,
                      )),
                    ),
                  ),
              ],
            ),
          )),
    );
  }
}

import 'dart:ui';

import 'package:flutter/material.dart';
import 'package:flutter_ai_app1/model.dart';
import 'package:get/get.dart';
import 'package:flutter_ai_app1/controller.dart';

class MyChatView extends StatelessWidget {
  MyChatView({super.key});
  var promptTextController = TextEditingController();
  // var chatAIController = Get.put(ChatAIController());
  var chatAIORTController = Get.put(ChatAIORTController());
  // late var chatHistory = chatAIORTController.chatTextHistory;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text("chat"),
      ),
      body: Column(children: [
        Expanded(
          child: GetBuilder<ChatAIORTController>(
            builder: (c) => ListView.builder(
                itemCount: c.chatTextHistory.value.length,
                itemBuilder: (context, index) {
                  return GetBuilder<ChatAIORTController>(
                      builder: (c2) => Text(c.chatTextHistory.value[index]));
                  //   title: Obx(
                  // () => Text("123"),
                  // c.chatTextHistory.value[index].value),
                }),
          ),
        ),
        Obx(() => Column(
              children: [Text(chatAIORTController.rxGeneratedText.value)],
            )),
        Row(
          children: [
            Expanded(
              child: TextFormField(
                decoration: const InputDecoration(
                  icon: Icon(Icons.person),
                  hintText: 'prompt : ask question',
                  labelText: 'User Input *',
                ),
                controller: promptTextController,
              ),
            ),
            ElevatedButton(
              onPressed: () {
                chatAIORTController.answerPrompt(promptTextController.text);
                // chatAIController.generateWhole(promptTextController.text);
                // chatAIController.generate(promptTextController.text);
              },
              child: Text("send"),
            )
          ],
        )
      ]),
    );
  }
}

// chatAIORTController.textStreamController.listen((e) {
//    generatedText += e;

//   });

class MyChatViewSTF extends StatefulWidget {
  MyChatViewSTF({super.key});

  @override
  State<MyChatViewSTF> createState() => _MyChatViewSTFState();
}

class _MyChatViewSTFState extends State<MyChatViewSTF> {
  var promptTextController = TextEditingController();
  var chatAIORTController = Get.put(ChatAIORTController());
  @override
  Widget build(BuildContext context) {
    // charaiORTtextStreamController.stream.listen((e) {
    //   print("init listen e : ${e}");
    // });
    // var stream1 = chatAIORTController.textStreamController.stream.listen((e) {
    //   setState(() {
    //     chatAIORTController.generatedText += e;
    //   });
    // });
    return Scaffold(
      appBar: AppBar(
        title: const Text("chat"),
      ),
      body: Column(children: [
        Expanded(
          child: GetBuilder<ChatAIORTController>(
            builder: (c) => ListView.builder(
                itemCount: c.chatTextHistory.value.length,
                itemBuilder: (context, index) {
                  return GetBuilder<ChatAIORTController>(
                      builder: (c2) => Text(c.chatTextHistory.value[index]));
                  //   title: Obx(
                  // () => Text("123"),
                  // c.chatTextHistory.value[index].value),
                }),
          ),
        ),
        Obx(() => Column(
              children: [Text(chatAIORTController.rxGeneratedText.value)],
            )),
        // StreamBuilder(
        //   stream: Stream.periodic(Duration(seconds: 1)),
        //   builder: (context, snapshot) => Text(
        //     "snapshot data: ${snapshot.data}",
        //   ),
        // ),
        StreamBuilder<String>(
          initialData: "input initialData.",

          // stream: chatAIORTController.chatAIORT
          //     .generateNextText(chatAIORTController.prompt_)
          //     .asBroadcastStream(),
          stream: chatAIORTController.textStreamController.stream,
          // stream: chatAIORTController.chatAIORT
          //     .generateNextText("what's your name?"),
          // chatAIORTController.textStreamController.stream,
          builder: (context, snapshot) {
            // if (snapshot.connectionState == ConnectionState.waiting) {
            //   return CircularProgressIndicator();
            // } else {
            if (snapshot.hasError) {
              print("snapshot has error.");
            }
            switch (snapshot.connectionState) {
              case ConnectionState.none:
                return Text("you can input your query.");
              case ConnectionState.waiting:
                return Center(child: CircularProgressIndicator());
              case ConnectionState.active:
                if (snapshot.hasData) {
                  return Flex(
                    direction: Axis.horizontal,
                    mainAxisAlignment: MainAxisAlignment.start,
                    children: [
                      Text(
                          // "snapshot.data! : ${snapshot.data!}, generatedText : ${chatAIORTController.generatedText}");
                          chatAIORTController.generatedText),
                    ],
                  );
                } else {
                  return Text("snapshot has no data.");
                }
              case ConnectionState.done:
                return Text(chatAIORTController.generatedText);
            }

            // if (snapshot.hasData) {
            //   chatAIORTController.generatedText += snapshot.data.toString();
            //   return Text(chatAIORTController.generatedText);
            // } else {
            //   return Text("snapshot no data.");
            // }
            // // }
          },
        ),
        Row(
          children: [
            Expanded(
              child: TextFormField(
                decoration: const InputDecoration(
                  icon: Icon(Icons.person),
                  hintText: 'prompt : ask question',
                  labelText: 'User Input *',
                ),
                controller: promptTextController,
              ),
            ),
            ElevatedButton(
              onPressed: () {
                chatAIORTController.answerPrompt(promptTextController.text);
                setState(() {});
                // chatAIController.generateWhole(promptTextController.text);
                // chatAIController.generate(promptTextController.text);
              },
              child: Text("send"),
            )
          ],
        )
      ]),
    );
  }
}

package com.example.kotlin_ai_app

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateMapOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.lifecycle.DefaultLifecycleObserver


class ChatAIController(var modelPath:String, var executionProvider:String)  : DefaultLifecycleObserver{
    private var nativeHandle by rememberSaveable { mutableStateOf(0L)}
    private external fun nativeCreateChatAI(modelPath : String, executionProvider : String) : Long
    private external fun nativeDeleteChatAI(nativeHandle : Long)
    private external fun nativeGenerateTextFromChatAI(nativeHandle : Long, prompt : String) : String
    private external fun nativeSetPrompt(nativeHandle : Long, prompt : String) : Boolean
    private external fun nativeIsDone(nativeHandle: Long) : Boolean
    private external fun nativeGenerateNext(nativeHandle: Long) : String
    var getLastError=""

    companion object{
        init {
            System.loadLibrary("my_chat_ai")
        }
    }

    fun createChatAI(){
        nativeHandle = nativeCreateChatAI(modelPath, executionProvider)
    }
    fun deleteChatAI(){
        if(nativeHandle!=0L){
            nativeDeleteChatAI(nativeHandle)
        }
    }

}

#include <string>

extern "C"{

__declspec(dllexport) void* create_chat_ai(const char* model_path, const char* execution_provider);

__declspec(dllexport) void delete_chat_ai(void* ptr);
__declspec(dllexport) const char* generate_text_from_chat_ai(void *ptr, const char* prompt);
__declspec(dllexport) bool set_prompt(void* ptr, const char* prompt);
__declspec(dllexport) bool is_done(void* ptr);
__declspec(dllexport) const char* generate_next(void* ptr);
}
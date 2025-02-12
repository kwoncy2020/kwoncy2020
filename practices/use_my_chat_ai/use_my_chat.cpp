#include "my_chat_ai.h"
#include <iostream>

int main(int argc, char** argv){
    auto p = create_chat_ai(argv[1],argv[2]);
    std::cout << "auto ptr: " << p << std::endl;
    std::string text;
    std::string s = generate_text_from_chat_ai(p,"what's your name?");
    std::cout << s << std::endl;
    std::cin.clear();
    while(true){
        std::cout << "Prompt: (type quit() to exit) Or (To terminate current output generation, press Ctrl+C)" << std::endl;
        std::getline(std::cin,text);
        if (text=="quit()") break;
        set_prompt(p,text.c_str());
        // s.clear();
        while (!is_done(p)){
            // s += std::string(generate_next(p));
            std::cout << generate_next(p);
        }
        std::cout << std::endl << std::endl;
    }

    // std::cout << s << std::endl;
    delete_chat_ai(p);
    return 0;
}
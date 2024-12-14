I think monitor brightness is important especially changed frequently.
My notebook has hotkey (fn + f11/f12) to control brightness.
Fisrt I try to make my custom hotkey and it will simulate prev hotkey(fn+f11/f12)
to chanege easily and quickly. With some try, I found it's almost impossible my
first try. Although I simulate correctly (fn+f11/f12), it's not work. Someone says it's kind of bios level's matter. 

Then, I try to modify brightness directly. It should be possible since I can control brightness through control panel on windows OS. I didn't imagine this took some time. The win32 api not work well. I figure out my code was not the problem. the problem was my notebook's driver or the manufacturer's not fulfilling.

Finally, I made program using wmi system in c++. There's another way you can use powershell or call powershell script in c++. I didn't use both of them. I use wmi object directly in c++.


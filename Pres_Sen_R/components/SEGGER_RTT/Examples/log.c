

#include <stdio.h>
#include "SEGGER_RTT.h"



//__DATE__ 当前日期，一个以 “MMM DD YYYY” 格式表示的字符串常量。（当前文件编译时的）
//__TIME__ 当前时间，一个以 “HH:MM:SS” 格式表示的字符串常量。（当前文件编译时的）
//__FILE__ 这会包含当前文件名，一个字符串常量。
//__LINE__ 这会包含当前行号，一个十进制常量。
//__STDC__ 当编译器以 ANSI 标准编译时，则定义为 1；判断该文件是不是标准 C 程序。



#define log_i(...) do{\
	if(LOG_TAG)  \
	SEGGER_RTT_printf(0,LOG_TAG " [" __TIME__ "] " __VA_ARGS__);\
	else \
	SEGGER_RTT_printf(0,__FILE__ " [" __TIME__ "] " __VA_ARGS__);\
}while(0)











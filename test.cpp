#include "xlogger.h"

#include <windows.h>
#include <stdio.h>
#include <locale.h>
int main() {
	xlogger_config conf = {0};
	conf.flag = XLOGGER_FLAG_USE_DIR | XLOGGER_FLAG_USE_CONSOLE;
	conf.dir = "mytest/log";
	xlogger_init(&conf);
	unsigned char utf8msg[] = { 0xe4,0xbd,0xa0,0xe5,0xa5,0xbd,0}; //ÄãºÃ
	printf("%s\n", conf.file);

	LOG(utf8msg);
	
	setlocale(LC_ALL, "");
		
	LOG(utf8msg);

	LOG("hello world 1");
	Sleep(1200);
	LOG("hello world 2");
	Sleep(1200);
	LOGE("error open file");
	
	return 0;
}
#include "xlogger.h"

#include <windows.h>
#include <stdio.h>

int main() {
	xlogger_config conf = {0};
	conf.flag = XLOGGER_FLAG_USE_DIR | XLOGGER_FLAG_USE_CONSOLE;
	conf.dir = "mytest/log";
	xlogger_init(&conf);

	printf("%s\n", conf.file);

	LOG("hello world 1");
	Sleep(1200);
	LOG("hello world 2");
	Sleep(1200);
	LOGE("error open file");
	return 0;
}
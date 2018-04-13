#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	/**
	极简文件日志库
	*/

	//日志等级
#define XLOGGER_LEVEL_DEBUG    -1
#define XLOGGER_LEVEL_INFO     0
#define XLOGGER_LEVEL_WARN     1
#define XLOGGER_LEVEL_ERROR    2

	//日志配置选项flag
#define XLOGGER_FLAG_USE_FILE  1 //指定日志文件  
#define XLOGGER_FLAG_USE_DIR   2 //指定日志目录
#define XLOGGER_FLAG_USE_CONSOLE   4 //同时打印到控制台
#define XLOGGER_FLAG_USE_DEBUGGER  8 //同时打印到调试器窗口

	/* 日志配置 */
	typedef struct {
		int level;  //日志等级
		int flag;  //选项
		const char* file; //指定文件
		const char* dir; //指定目录 
		int maxday;  //最长保留天数
	}xlogger_config;


	/** 初始化日志 */
	int xlogger_init(xlogger_config* conf);
	
	/** 写日志 */
	void xlogger_write(int level, const char* msg);


//便捷使用宏
#define LOGD(msg)  xlogger_write(XLOGGER_LEVEL_DEBUG, (char*)(msg))
#define LOGI(msg)  xlogger_write(XLOGGER_LEVEL_INFO, (char*)(msg))
#define LOGW(msg)  xlogger_write(XLOGGER_LEVEL_WARN,(char*)(msg))
#define LOGE(msg)  xlogger_write(XLOGGER_LEVEL_ERROR, (char*)(msg))
#define LOG(msg) LOGI(msg)

#ifdef __cplusplus
}
#endif



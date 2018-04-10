#include "xlogger.h"

#include <string>
#include <filesystem>
#include <chrono>
#include <random>
#include <time.h>
#include <string.h>
#include <stdlib.h>



using namespace std;
using namespace std::chrono;
namespace fs = std::experimental::filesystem::v1;


static int loginited_;
static int loglevel_;
static int logflag_;
static string logfile_;
static string logdir_;
static FILE* fp_;
static string lastdate_; //最后一条日志日期
static int maxday_;


//格式化当前时间
static string format_now(const char* fmt) {
	string ret;
	char timebuf[64];
	time_t nowt = time(0);
	struct tm nowtm = { 0 };
	localtime_s(&nowtm, &nowt);
	timebuf[0] = 0;
	strftime(timebuf, 64, fmt, &nowtm);
	ret = timebuf;
	return ret;
}


//当前日期字符串
static string format_now_date() {
	return format_now("%Y-%m-%d");
}

//当前时间字符串
static string format_now_time() {
	return format_now("%H:%M:%S");
}


//随机字符串
static string random_string(size_t len, const string& base="") {
	static string defaultbase = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	string result;
	if (len) {
		result.reserve(len + 1);
		string usebase = base.size() ? base : defaultbase;
		std::default_random_engine e;
		std::uniform_int_distribution<> u(0, usebase.size() - 1);
		for (size_t i = 0; i < len; i++) {
			if (i % 15 == 0) {
				std::random_device rd;
				e.seed(rd());
			}
			size_t index = u(e);
			result.append(1, usebase.at(index));
		}
	}
	return result;
}




//创建不重名日志文件
static string xlogger_gen_unique_path(fs::path dir) {
	fs::path ret;
	error_code e;
	string name;
	string nowstr = format_now("%Y%m%d%H%M%S");
	while (true) {
		name = nowstr + random_string(4) + ".log";
		ret = dir / name;		
		if (!fs::exists(ret)) {			
			break;
		}
	}
	//logfile_ = ret.u8string();
	//fp_ = logger_open(ret);
	return ret.u8string();
}


//删除日期比较早的文件
static void xlogger_clear_old_file() {
	fs::path dir = fs::u8path(logdir_);	
	error_code e;
	auto now = system_clock::now();
	for (auto entry : fs::directory_iterator(dir)){
		fs::path p = entry.path();
		if (fs::is_regular_file(p) && p.extension() == ".log" ) {
			auto t = fs::last_write_time(p, e);
			if (!e) {
				auto duration = duration_cast<hours>(now - t);
				if (duration.count() > (maxday_ * 24)) {
					fs::remove(p, e);
				}
			}
		}
	}	
}

//格式化日志字符串
static string format_log(int level, const char* msg) {
	string ret;
	char timebuf[64];
	time_t nowt = time(0);
	struct tm nowtm = {0};
	localtime_s(&nowtm, &nowt);
	timebuf[0] = 0;
	strftime(timebuf, 64, "%H:%M:%S", &nowtm);

	ret += timebuf;
	if (level < XLOGGER_LEVEL_DEBUG)level = XLOGGER_LEVEL_DEBUG;
	if (level > XLOGGER_LEVEL_ERROR)level = XLOGGER_LEVEL_ERROR;
	switch (level)	{
	case XLOGGER_LEVEL_DEBUG:ret += " D "; break;
	case XLOGGER_LEVEL_INFO:ret += " I "; break;
	case XLOGGER_LEVEL_WARN:ret += " W "; break;
	case XLOGGER_LEVEL_ERROR:ret += " E "; break;
	default:
		ret += "   ";
		break;
	}
	
	if (msg)ret += msg;
	ret += "\r\n";
	return ret;
}


//打开日志文件
static FILE* logger_open(const fs::path& p) {
	FILE* f = NULL;
	_wfopen_s(&f, p.c_str(), L"wb");

	//每次打开新文件时，写入当前日期
	/*
	if (f) {
		string today = format_now_date();
		string data = format_log(0, today.c_str());
		fwrite(data.c_str(), 1, data.length(), f);
		fflush(f);
	}*/

	return f;
}


//写文件前处理, 重命名每天文件
static void befor_write_file_log() {

	if (!(logflag_ & XLOGGER_FLAG_USE_DIR))return; 

	//只有输出到目录的方式需要处理
	string today = format_now_date();
	if (lastdate_.size() && today != lastdate_) { //已经第二天了
		xlogger_clear_old_file();
		error_code e;
		//创建新的日志文件
		fs::path p = fs::u8path(logfile_);
		fs::path ext = today;
		ext += ".log";
		fs::path newp = p;
		newp.replace_extension(ext);
		
		if (fp_)fclose(fp_);
		fs::rename(p, newp, e);
		fp_ = logger_open(p);
	}
}



static void xlogger_init_dir() {
	fs::path dir = fs::u8path(logdir_); 
	error_code e;
	fs::create_directories(dir,e);
	xlogger_clear_old_file();

	logfile_ = xlogger_gen_unique_path(dir);	
	logflag_ |= XLOGGER_FLAG_USE_FILE;

	fs::path f = fs::u8path(logfile_);
	fp_ = logger_open(f);
}

static void xlogger_init_file() {
	fs::path f = fs::u8path(logfile_);
	fp_ = logger_open(f);
}

/** 初始化日志 */
int xlogger_init(xlogger_config* conf) {
	int ret = 0;
	if (loginited_)return 0;
	loginited_ = 1;
	maxday_ = 10;
		
	if (!conf ||!conf->flag) {
		loglevel_ = -9999;
		logflag_ = XLOGGER_FLAG_USE_CONSOLE;		
	}
	else {
		loglevel_ = conf->level;
		logflag_ = conf->flag;
		if(conf->maxday>0)maxday_ = conf->maxday;
		if (logflag_ & XLOGGER_FLAG_USE_DIR) {			
			logdir_ = conf->dir?conf->dir:"log";
			xlogger_init_dir();
			conf->file = logfile_.c_str();
		}else if (logflag_ & XLOGGER_FLAG_USE_FILE) {
			logfile_ = conf->file? conf->file:"app.log";
			xlogger_init_file();
		}
	}
	return ret;
}

/** 写日志 */
void xlogger_write(int level, const char* msg) {
	if (level < loglevel_)return;
	string data = format_log(level, msg);
	if (logflag_ & XLOGGER_FLAG_USE_CONSOLE){
		//输出到控制台
		printf("%s", data.c_str());
	}
	if (logflag_ & XLOGGER_FLAG_USE_DEBUGGER) {		
		//TODO 输出到调试器
	}
	if (logflag_ & XLOGGER_FLAG_USE_FILE) {
		//输出到文件
		befor_write_file_log();
		if (fp_) {
			fwrite(data.c_str(), 1, data.length(), fp_);
			fflush(fp_);
		}
	}
	lastdate_ = format_now_date();
}



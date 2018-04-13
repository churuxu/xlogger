#include "xlogger.h"

#include <string>
#include <filesystem>
#include <chrono>
#include <random>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
#endif


using namespace std;
using namespace std::chrono;
namespace fs = std::experimental::filesystem::v1;


static int loginited_;
static int loglevel_;
static int logflag_;
static string logfile_;
static string logdir_;
static FILE* fp_;
static string lastdate_; //���һ����־����
static int maxday_;


//��ʽ����ǰʱ��
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


//��ǰ�����ַ���
static string format_now_date() {
	return format_now("%Y-%m-%d");
}

//��ǰʱ���ַ���
static string format_now_time() {
	return format_now("%H:%M:%S");
}


//����ַ���
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




//������������־�ļ�
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


//ɾ�����ڱȽ�����ļ�
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

//��ʽ����־�ַ���
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


//����־�ļ�
static FILE* logger_open(const fs::path& p) {
	FILE* f = NULL;
	_wfopen_s(&f, p.c_str(), L"wb");

	//ÿ�δ����ļ�ʱ��д�뵱ǰ����
	/*
	if (f) {
		string today = format_now_date();
		string data = format_log(0, today.c_str());
		fwrite(data.c_str(), 1, data.length(), f);
		fflush(f);
	}*/

	return f;
}


//д�ļ�ǰ����, ������ÿ���ļ�
static void befor_write_file_log() {

	if (!(logflag_ & XLOGGER_FLAG_USE_DIR))return; 

	//ֻ�������Ŀ¼�ķ�ʽ��Ҫ����
	string today = format_now_date();
	if (lastdate_.size() && today != lastdate_) { //�Ѿ��ڶ�����
		xlogger_clear_old_file();
		error_code e;
		//�����µ���־�ļ�
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

/** ��ʼ����־ */
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


static void write_to_file(const string& data) {
	befor_write_file_log();
	if (fp_) {
		fwrite(data.c_str(), 1, data.length(), fp_);
		fflush(fp_);
	}
}

#ifdef _WIN32

static void write_to_console(const string& data) {
	int sz = data.length() * 2 + 4;
	wchar_t* buf = (wchar_t*)malloc(sz);
	DWORD wrt = 0;
	buf[0] = 0;
	int len = MultiByteToWideChar(CP_UTF8, 0, data.c_str(), data.length(), buf, sz);
	if (len > 0)buf[len] = 0;
	//wprintf(L"%s", buf);
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), buf, len, &wrt, NULL);
	free(buf);
}

static void write_to_debugger(const string& data) {
	int sz = data.length() * 2 + 4;
	wchar_t* buf = (wchar_t*)malloc(sz);
	buf[0] = 0;
	int len = MultiByteToWideChar(CP_UTF8, 0, data.c_str(), data.length(), buf, sz);
	if (len > 0)buf[len] = 0;
	OutputDebugStringW(buf);
	free(buf);	
}

#else

static void write_to_console(const string& data) {
	printf("%s", data.c_str());
}

static void write_to_debugger(const string& data) {

}
#endif


/** д��־ */
void xlogger_write(int level, const char* msg) {
	if (level < loglevel_)return;
	string data = format_log(level, msg);
	if (logflag_ & XLOGGER_FLAG_USE_CONSOLE){
		//���������̨
		write_to_console(data);
	}
	if (logflag_ & XLOGGER_FLAG_USE_DEBUGGER) {		
		//�����������
		write_to_debugger(data);
	}
	if (logflag_ & XLOGGER_FLAG_USE_FILE) {
		//������ļ�
		write_to_file(data);
	}
	lastdate_ = format_now_date();
}



#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	/**
	�����ļ���־��
	*/

	//��־�ȼ�
#define XLOGGER_LEVEL_DEBUG    -1
#define XLOGGER_LEVEL_INFO     0
#define XLOGGER_LEVEL_WARN     1
#define XLOGGER_LEVEL_ERROR    2

	//��־����ѡ��flag
#define XLOGGER_FLAG_USE_FILE  1 //ָ����־�ļ�  
#define XLOGGER_FLAG_USE_DIR   2 //ָ����־Ŀ¼
#define XLOGGER_FLAG_USE_CONSOLE   4 //ͬʱ��ӡ������̨
#define XLOGGER_FLAG_USE_DEBUGGER  8 //ͬʱ��ӡ������������

	/* ��־���� */
	typedef struct {
		int level;  //��־�ȼ�
		int flag;  //ѡ��
		const char* file; //ָ���ļ�
		const char* dir; //ָ��Ŀ¼ 
		int maxday;  //���������
	}xlogger_config;


	/** ��ʼ����־ */
	int xlogger_init(xlogger_config* conf);
	
	/** д��־ */
	void xlogger_write(int level, const char* msg);


//���ʹ�ú�
#define LOGD(msg)  xlogger_write(XLOGGER_LEVEL_DEBUG, (char*)(msg))
#define LOGI(msg)  xlogger_write(XLOGGER_LEVEL_INFO, (char*)(msg))
#define LOGW(msg)  xlogger_write(XLOGGER_LEVEL_WARN,(char*)(msg))
#define LOGE(msg)  xlogger_write(XLOGGER_LEVEL_ERROR, (char*)(msg))
#define LOG(msg) LOGI(msg)

#ifdef __cplusplus
}
#endif



#pragma once
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/time.h>

#define LOG(lev, msg) Log(lev, __FILE__, __LINE__, msg)

//[时间 info/warning/error/fatal/debug 文件 行号] 具体的错误信息
const char* Level[] = { "INFO", "WARNING", "ERROR", "FATAL", "DEBUG" };

enum LogLevel {
    INFO = 0,
    WARNING,
    ERROR,
    FATAL,
    DEBUG
};

class LogTime {
public:
    static void GetTimeStamp(std::string& timestamp) {
        time_t SysTime;
        //获取时间戳
        time(&SysTime);

        //将时间戳转化为年月日时分秒
        struct tm* ST = localtime(&SysTime);
        //格式化字符串 [YYYY-MM-DD HH-mm-SS]
        char TimeNow[23] = {'\0'};
        snprintf(TimeNow, sizeof(TimeNow) - 1, "%04d-%02d-%02d %02d:%02d:%02d", ST->tm_year + 1900, ST->tm_mon + 1, ST->tm_mday, ST->tm_hour, ST->tm_min, ST->tm_sec);
        timestamp.assign(TimeNow, strlen(TimeNow));
    }
};

//这里的inline的必要性：是要将代码展开，不然在其他地方调用展开的文件名和行号都是该文件
inline void Log(LogLevel lev, const char* file, int line, const std::string& logmsg) {
    std::string level_info = Level[lev];
    std::string timer_stamp;

    LogTime::GetTimeStamp(timer_stamp);

    //[时间 info/warning/error/fatal/debug 文件 行号] 具体的错误信息
    std::cout << "[" << timer_stamp << " " << level_info << " " << file << ":" <<
        line << "]" << logmsg << std::endl;
}

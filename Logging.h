#ifndef _LOGGING_H_
#define _LOGGING_H_

void LogIt(const char* fn, const char* file, int line,
           const char* message = "");

#define LOGIT \
  { LogIt(__FUNCTION__, __FILE__, __LINE__); }
#define LOGITM(msg) \
  { LogIt(__FUNCTION__, __FILE__, __LINE__, (msg)); }
//#define LOGIT	{}
//#define LOGITM	{}

void CloseLogFile();

#endif

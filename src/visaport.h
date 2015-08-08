
#ifndef _visaport_h_
#define _visaport_h_

#include <nan.h>
#include <list>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_STRING_SIZE 1024
#define QUERY_STRING_SIZE 1024

NAN_METHOD(List);
void EIO_List(uv_work_t* req);
void EIO_AfterList(uv_work_t* req);

NAN_METHOD(Open);
void EIO_Open(uv_work_t* req);
void EIO_AfterOpen(uv_work_t* req);
void AfterOpenSuccess(int fd);

NAN_METHOD(Query);
void EIO_Query(uv_work_t* req);
void EIO_AfterQuery(uv_work_t* req);

NAN_METHOD(Close);
void EIO_Close(uv_work_t* req);
void EIO_AfterClose(uv_work_t* req);

struct ListResultItem {
public:
  std::string path;
  std::string idn;
};

struct ListBaton {
public:
  NanCallback* callback;
  std::list<ListResultItem*> results;
  char errorString[ERROR_STRING_SIZE];
};

struct OpenBaton {
public:
  char path[1024];
  NanCallback* callback;
  int result;
  char errorString[ERROR_STRING_SIZE];
};

struct QueryBaton {
public:
  char cmd[QUERY_STRING_SIZE];
  
  NanCallback* callback;
  //std::list<ListResultItem*> results;
  char result[QUERY_STRING_SIZE];
  char errorString[ERROR_STRING_SIZE];
};

struct CloseBaton {
public:
  int fd;
  NanCallback* callback;
  char errorString[ERROR_STRING_SIZE];
};

#endif

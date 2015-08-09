
#ifndef _nisa32c_h_
#define _nisa32c_h_

#include <nan.h>
#include <list>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <visa.h>

#define ERROR_STRING_SIZE 1024

NAN_METHOD(List);
void EIO_List(uv_work_t* req);
void EIO_AfterList(uv_work_t* req);

struct ListResultItem {
public:
  std::string path;
  std::string idn;
};

#endif

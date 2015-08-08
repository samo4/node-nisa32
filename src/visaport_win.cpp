#include "visaport.h"
#include <list>
#include "win/disphelper.h"

#include "win/stdafx.h"
#include "win/enumser.h"

#include "visa.h"

#include <nan.h>

#ifdef WIN32

#define MAX_BUFFER_SIZE 1000

// Declare type of pointer to CancelIoEx function
typedef BOOL (WINAPI *CancelIoExType)(HANDLE hFile, LPOVERLAPPED lpOverlapped);

ViSession defaultRM, instr;


std::list<int> g_closingHandles;
int bufferSize;
void ErrorCodeToString(const char* prefix, int errorCode, char *errorStr) {
  switch(errorCode) {
  default:
    _snprintf(errorStr, ERROR_STRING_SIZE, "%s: Unknown error code %d", prefix, errorCode);
    break;
  }
}

void EIO_Open(uv_work_t* req) {
  OpenBaton* data = static_cast<OpenBaton*>(req->data);
  ViStatus status;
  char temp[1024];
  
  status = viOpenDefaultRM(&defaultRM);
  if (status < VI_SUCCESS) {
    _snprintf(temp, sizeof(temp), "Opening RM %s", data->path);
    ErrorCodeToString(temp, status, data->errorString);
    return;
  }
  status = viOpen(defaultRM, data->path, VI_NULL, VI_NULL, &instr);
  if (status < VI_SUCCESS) {
    _snprintf(temp, sizeof(temp), "Opening session %s", data->path);
    ErrorCodeToString(temp, status, data->errorString);
    return;
  }
  data->result = status;
}

void EIO_Query(uv_work_t* req) {
	QueryBaton* data = static_cast<QueryBaton*>(req->data);
	char temp[QUERY_STRING_SIZE];
	ViStatus status;
  
	if (instr < 1) {
		ErrorCodeToString("not open", 11, data->errorString);
		return;
	}
	status = viQueryf(instr, data->cmd,"%s",temp);
	if ((status < VI_SUCCESS)) {
		_snprintf(temp, sizeof(temp), "");
		ErrorCodeToString(temp, status, data->errorString);
		return;
	}
	_snprintf(data->result, QUERY_STRING_SIZE, "%s", temp);
}

void EIO_List(uv_work_t* req) {
  ListBaton* data = static_cast<ListBaton*>(req->data);
  
  char temp[100];
  ViUInt16 iManf;
  ViChar desc[VI_FIND_BUFLEN];
  ViUInt32 numInstrs;
  ViFindList fList;
  ViStatus status;
  ViSession defaultRM, instr;
  
  status = viOpenDefaultRM(&defaultRM);
  if (status < VI_SUCCESS) {
    _snprintf(temp, sizeof(temp), "LIST: Opening RM");
    ErrorCodeToString(temp, status, data->errorString);
    return;
  }
  
  status = viFindRsrc(defaultRM, "?*INSTR", &fList, &numInstrs, desc); 
  if (status < VI_SUCCESS) {
    _snprintf(temp, sizeof(temp), "viFindRsrc1");
    ErrorCodeToString(temp, status, data->errorString);
    return;
  }
  
  while (numInstrs--) {
    
	status = viOpen(defaultRM, desc, VI_NULL, VI_NULL, &instr);
	if (status < VI_SUCCESS) {
		viFindNext(fList, desc);
		continue;
	}
	
	char temp[256];
	status = viQueryf(instr,"*IDN?\n","%s",temp);
	if ((status < VI_SUCCESS)) {
		viClose(instr);
		_snprintf(temp, sizeof(temp), "");
	}
  if (temp[0] == 0) {
    status = viQueryf(instr,"ID?\n","%s",temp);
  	if ((status < VI_SUCCESS)) {
  		viClose(instr);
  		_snprintf(temp, sizeof(temp), "");
  	} 
  }
  ListResultItem* resultItem = new ListResultItem();
	resultItem->path = desc;
	resultItem->idn = temp;
	data->results.push_back(resultItem);
	viFindNext(fList, desc);
  }
  viClose(fList);
  // viClose(defaultRM);
}

void EIO_Close(uv_work_t* req) {
  CloseBaton* data = static_cast<CloseBaton*>(req->data);

  g_closingHandles.push_back(data->fd);
  
  viClose(instr);
  viClose(defaultRM);
}

#endif

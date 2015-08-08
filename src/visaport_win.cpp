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
  
  ViUInt16 iManf;
  ViChar desc[VI_FIND_BUFLEN];
  ViUInt32 numInstrs;
  ViFindList fList;
  ViStatus status;
  ViSession defaultRM, instr;
  if (defaultRM < 1) {
	  status = viOpenDefaultRM(&defaultRM);
	  if (status < VI_SUCCESS) {
		char temp[100];
		_snprintf(temp, sizeof(temp), "viOpenDefaultRM");
		ErrorCodeToString(temp, status, data->errorString);
		return;
	  }
  }
  status = viFindRsrc(defaultRM, "?*INSTR\n", &fList, &numInstrs, desc);
  if (status < VI_SUCCESS) {
    char temp[100];
    _snprintf(temp, sizeof(temp), "viFindRsrc");
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
    ListResultItem* resultItem = new ListResultItem();
	resultItem->path = desc;
	resultItem->idn = temp;
	data->results.push_back(resultItem);
	viFindNext(fList, desc);
  }
  viClose(fList);
  // viClose(defaultRM);
}

struct WatchPortBaton {
public:
  HANDLE fd;
  DWORD bytesRead;
  char buffer[MAX_BUFFER_SIZE];
  char errorString[ERROR_STRING_SIZE];
  DWORD errorCode;
  bool disconnected;
  NanCallback* dataCallback;
  NanCallback* errorCallback;
  NanCallback* disconnectedCallback;
};

void EIO_WatchPort(uv_work_t* req) {
  WatchPortBaton* data = static_cast<WatchPortBaton*>(req->data);
  data->bytesRead = 0;
  data->disconnected = false;

  // Event used by GetOverlappedResult(..., TRUE) to wait for incoming data or timeout
  // Event MUST be used if program has several simultaneous asynchronous operations
  // on the same handle (i.e. ReadFile and WriteFile)
  HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

  while(true) {
    OVERLAPPED ov = {0};
    ov.hEvent = hEvent;

    // Start read operation - synchrounous or asynchronous
    DWORD bytesReadSync = 0;
    if(!ReadFile((HANDLE)data->fd, data->buffer, bufferSize, &bytesReadSync, &ov)) {
      data->errorCode = GetLastError();
      if(data->errorCode != ERROR_IO_PENDING) {
        // Read operation error
        if(data->errorCode == ERROR_OPERATION_ABORTED) {
          data->disconnected = true;
        }
        else {
          ErrorCodeToString("Reading from COM port (ReadFile)", data->errorCode, data->errorString);
        }
        break;
      }

      // Read operation is asynchronous and is pending
      // We MUST wait for operation completion before deallocation of OVERLAPPED struct
      // or read data buffer

      // Wait for async read operation completion or timeout
      DWORD bytesReadAsync = 0;
      if(!GetOverlappedResult((HANDLE)data->fd, &ov, &bytesReadAsync, TRUE)) {
        // Read operation error
        data->errorCode = GetLastError();
        if(data->errorCode == ERROR_OPERATION_ABORTED) {
          data->disconnected = true;
        }
        else {
          ErrorCodeToString("Reading from COM port (GetOverlappedResult)", data->errorCode, data->errorString);
        }
        break;
      }
      else {
        // Read operation completed asynchronously
        data->bytesRead = bytesReadAsync;
      }
    }
    else {
      // Read operation completed synchronously
      data->bytesRead = bytesReadSync;
    }

    // Return data received if any
    if(data->bytesRead > 0) {
      break;
    }
  }

  CloseHandle(hEvent);
}

bool IsClosingHandle(int fd) {
  for(std::list<int>::iterator it=g_closingHandles.begin(); it!=g_closingHandles.end(); ++it) {
    if(fd == *it) {
      g_closingHandles.remove(fd);
      return true;
    }
  }
  return false;
}

void DisposeWatchPortCallbacks(WatchPortBaton* data) {
  delete data->dataCallback;
  delete data->errorCallback;
  delete data->disconnectedCallback;
}

void EIO_AfterWatchPort(uv_work_t* req) {
  NanScope();

  WatchPortBaton* data = static_cast<WatchPortBaton*>(req->data);
  if(data->disconnected) {
    data->disconnectedCallback->Call(0, NULL);
    DisposeWatchPortCallbacks(data);
    goto cleanup;
  }

  if(data->bytesRead > 0) {
    v8::Handle<v8::Value> argv[1];
    argv[0] = NanNewBufferHandle(data->buffer, data->bytesRead);
    data->dataCallback->Call(1, argv);
  } else if(data->errorCode > 0) {
    if(data->errorCode == ERROR_INVALID_HANDLE && IsClosingHandle((int)data->fd)) {
      DisposeWatchPortCallbacks(data);
      goto cleanup;
    } else {
      v8::Handle<v8::Value> argv[1];
      argv[0] = NanError(data->errorString);
      data->errorCallback->Call(1, argv);
      Sleep(100); // prevent the errors from occurring too fast
    }
  }
  AfterOpenSuccess((int)data->fd);

cleanup:
  delete data;
  delete req;
}

void AfterOpenSuccess(int fd) {
  WatchPortBaton* baton = new WatchPortBaton();
  memset(baton, 0, sizeof(WatchPortBaton));
  baton->fd = (HANDLE)fd;
  /*baton->dataCallback = dataCallback;
  baton->errorCallback = errorCallback;
  baton->disconnectedCallback = disconnectedCallback;*/

  uv_work_t* req = new uv_work_t();
  req->data = baton;

  uv_queue_work(uv_default_loop(), req, EIO_WatchPort, (uv_after_work_cb)EIO_AfterWatchPort);
}

void EIO_Close(uv_work_t* req) {
  CloseBaton* data = static_cast<CloseBaton*>(req->data);

  g_closingHandles.push_back(data->fd);
  
  viClose(instr);
  viClose(defaultRM);
}

#endif

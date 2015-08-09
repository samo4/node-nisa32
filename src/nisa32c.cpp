/*********************************************************************
 * original Copyright (c) 2015 NAN contributors
 ********************************************************************/
#include <nan.h>
#include <list>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nisa32c.h"

using namespace Nan;  // NOLINT(build/namespaces)


void ErrorCodeToString(const char* prefix, int errorCode, char *errorStr) {
  switch(errorCode) {
  default:
    _snprintf(errorStr, ERROR_STRING_SIZE, "%s: Unknown error code %d", prefix, errorCode);
    break;
  }
}

class Nisa32c : public node::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init);

 private:
  Nisa32c();
  ~Nisa32c();

  static NAN_METHOD(List);
  static NAN_METHOD(New);
  static NAN_METHOD(CallEmit);
  static Persistent<v8::Function> constructor;
};

Persistent<v8::Function> Nisa32c::constructor;

Nisa32c::Nisa32c() {
}

Nisa32c::~Nisa32c() {
}

NAN_MODULE_INIT(Nisa32c::Init) {
  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New<v8::String>("Nisa32c").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  SetPrototypeMethod(tpl, "call_emit", CallEmit);

  constructor.Reset(tpl->GetFunction());
  Set(target, Nan::New("Nisa32c").ToLocalChecked(), tpl->GetFunction());
}


// temporarily here until it compiles
void EIO_List(uv_work_t* req) {
  // ListBaton* data = static_cast<ListBaton*>(req->data);
  
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
    //ErrorCodeToString(temp, status, data->errorString);
    return;
  }
  
  status = viFindRsrc(defaultRM, "?*INSTR", &fList, &numInstrs, desc); 
  if (status < VI_SUCCESS) {
    _snprintf(temp, sizeof(temp), "viFindRsrc1");
    //ErrorCodeToString(temp, status, data->errorString);
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
  /*ListResultItem* resultItem = new ListResultItem();
	resultItem->path = desc;
	resultItem->idn = temp;
	data->results.push_back(resultItem);*/
	viFindNext(fList, desc);
  }
  viClose(fList);
  // viClose(defaultRM);
}

NAN_METHOD(Nisa32c::List) {
  Nan::HandleScope scope;

  // callback
  if(!info[0]->IsFunction()) {
	return Nan::ThrowError("First argument must be a function");
  }
  
  /*
  v8::Local<v8::Function> callback = info[0].As<v8::Function>();

  ListBaton* baton = new ListBaton();
  strcpy(baton->errorString, "");
  baton->callback = new NanCallback(callback);

  uv_work_t* req = new uv_work_t();
  req->data = baton;
  uv_queue_work(uv_default_loop(), req, EIO_List, (uv_after_work_cb)EIO_AfterList);
	
  */
  info.GetReturnValue().SetUndefined();
}

NAN_METHOD(Nisa32c::New) {
  if (info.IsConstructCall()) {
    Nisa32c* obj = new Nisa32c();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
    info.GetReturnValue().Set(cons->NewInstance());
  }
}

NAN_METHOD(Nisa32c::CallEmit) {
  v8::Local<v8::Value> argv[1] = {
    Nan::New("event").ToLocalChecked(),  // event name
  };

  MakeCallback(info.This(), "emit", 1, argv);
  info.GetReturnValue().SetUndefined();
}

NODE_MODULE(makecallback, Nisa32c::Init)
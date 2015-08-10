#include <nan.h>)
#include "visaList.h"
#include <cstring>

using v8::Function;
using v8::Local;
using v8::Number;
using v8::Value;
using Nan::AsyncQueueWorker;
using Nan::AsyncWorker;
using Nan::Callback;
using Nan::HandleScope;
using Nan::New;
using Nan::Null;
using Nan::To;
using Nan::Persistent;


class VisaListWorker : public AsyncWorker {
 public:
  VisaListWorker(Callback *callback, Persistent<v8::String> s) : AsyncWorker(callback), query(s) {}
  ~VisaListWorker() {}
  
  // Executed inside the worker-thread.
  // It is not safe to access V8, or V8 data structures
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute () {
    // estimate = Estimate(points);
  }
  
  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use V8 again
  void HandleOKCallback () {
    HandleScope scope;

    Local<Value> argv[] = {
        Null()
      /*, New<Number>(estimate)*/
    };

    callback->Call(1, argv);
  }
  
  private:
  	Persistent<v8::String> query;
  /*
  	std::string result;*/
};

NAN_METHOD(VisaList) {
  Persistent<v8::String> query(info[0].As<v8::String>());
  
  //std::string query = To<std::string>(info[0]).FromJust();
  Callback *callback = new Callback(info[1].As<Function>());

  AsyncQueueWorker(new VisaListWorker(callback, query));
}


/*
NAN_METHOD(Nisa32c::List) {
  Nan::HandleScope scope;

  // callback
  if(!info[0]->IsFunction()) {
	return Nan::ThrowError("First argument must be a function");
  }
  
  
  v8::Local<v8::Function> callback;
  callback = info[0].As<v8::Function>();
  ListBaton* baton = new ListBaton();
  baton->callback = new Nan::Callback(callback);
    Nan::Callback callback;
  
  v8::Local<v8::Function> callback = info[0].As<v8::Function>();

  ListBaton* baton = new ListBaton();
  strcpy(baton->errorString, "");
  baton->callback = new NanCallback(callback);

  uv_work_t* req = new uv_work_t();
  req->data = baton;
  uv_queue_work(uv_default_loop(), req, EIO_List, (uv_after_work_cb)EIO_AfterList);
	
 
  info.GetReturnValue().SetUndefined();
  
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
} */
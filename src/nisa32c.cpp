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
#include "visaList.h"

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

NAN_MODULE_INIT(Nisa32c::Init) {
  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New<v8::String>("Nisa32c").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  SetPrototypeMethod(tpl, "call_emit", CallEmit);

  constructor.Reset(tpl->GetFunction());
  Set(target, Nan::New("Nisa32c").ToLocalChecked(), tpl->GetFunction());
  
  // Set(target, Nan::New<v8::String>("list").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(VisaList)->GetFunction());
}

NODE_MODULE(makecallback, Nisa32c::Init)
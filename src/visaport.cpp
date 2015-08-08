#include "visaport.h"

#ifdef WIN32
#define strncasecmp strnicmp
#endif

uv_mutex_t write_queue_mutex;

NAN_METHOD(Open) {
  NanScope();

  uv_mutex_init(&write_queue_mutex);

  // path
  if(!args[0]->IsString()) {
    NanThrowTypeError("First argument must be a string");
    NanReturnUndefined();
  }
  v8::String::Utf8Value path(args[0]->ToString());

  // callback
  if(!args[1]->IsFunction()) {
    NanThrowTypeError("Second argument must be a function");
    NanReturnUndefined();
  }
  v8::Local<v8::Function> callback = args[1].As<v8::Function>();

  OpenBaton* baton = new OpenBaton();
  memset(baton, 0, sizeof(OpenBaton));
  strcpy(baton->path, *path);
  baton->callback = new NanCallback(callback);

  uv_work_t* req = new uv_work_t();
  req->data = baton;

  uv_queue_work(uv_default_loop(), req, EIO_Open, (uv_after_work_cb)EIO_AfterOpen);

  NanReturnUndefined();
}

void EIO_AfterOpen(uv_work_t* req) {
  NanScope();

  OpenBaton* data = static_cast<OpenBaton*>(req->data);

  v8::Handle<v8::Value> argv[2];
  if(data->errorString[0]) {
    argv[0] = v8::Exception::Error(NanNew<v8::String>(data->errorString));
    argv[1] = NanUndefined();
  } else {
    argv[0] = NanUndefined();
    argv[1] = NanNew<v8::Int32>(data->result);
  }

  data->callback->Call(2, argv);

  delete data->callback;
  delete data;
  delete req;
}

NAN_METHOD(Query) {
  NanScope();
  if(!args[0]->IsString()) {
    NanThrowTypeError("First argument must be a string");
    NanReturnUndefined();
  }
  v8::String::Utf8Value cmd(args[0]->ToString());
  
  if(!args[1]->IsFunction()) {
    NanThrowTypeError("Second argument must be a function: err, res");
    NanReturnUndefined();
  }
  v8::Local<v8::Function> callback = args[1].As<v8::Function>();

  QueryBaton* baton = new QueryBaton();
  memset(baton, 0, sizeof(QueryBaton));
  strcpy(baton->errorString, "");
  strcpy(baton->cmd, *cmd);
  baton->callback = new NanCallback(callback);
  uv_work_t* req = new uv_work_t();
  req->data = baton;
  uv_queue_work(uv_default_loop(), req, EIO_List, (uv_after_work_cb)EIO_AfterList);

  NanReturnUndefined();
}

void EIO_AfterQuery(uv_work_t* req) {
  NanScope();

  QueryBaton* data = static_cast<QueryBaton*>(req->data);

  v8::Handle<v8::Value> argv[2];
  if(data->errorString[0]) {
    argv[0] = v8::Exception::Error(NanNew<v8::String>(data->errorString));
    argv[1] = NanUndefined();
  } else {
    argv[0] = NanUndefined();
    argv[1] = NanNew(data->result);
  }
  data->callback->Call(2, argv);

  delete data->callback;
  delete data;
  delete req;
}

NAN_METHOD(Close) {
  NanScope();

  // file descriptor
  if(!args[0]->IsInt32()) {
    NanThrowTypeError("First argument must be an int");
    NanReturnUndefined();
  }
  int fd = args[0]->ToInt32()->Int32Value();

  // callback
  if(!args[1]->IsFunction()) {
    NanThrowTypeError("Second argument must be a function");
    NanReturnUndefined();
  }
  v8::Local<v8::Function> callback = args[1].As<v8::Function>();

  CloseBaton* baton = new CloseBaton();
  memset(baton, 0, sizeof(CloseBaton));
  baton->fd = fd;
  baton->callback = new NanCallback(callback);

  uv_work_t* req = new uv_work_t();
  req->data = baton;
  uv_queue_work(uv_default_loop(), req, EIO_Close, (uv_after_work_cb)EIO_AfterClose);

  NanReturnUndefined();
}

void EIO_AfterClose(uv_work_t* req) {
  NanScope();

  CloseBaton* data = static_cast<CloseBaton*>(req->data);

  v8::Handle<v8::Value> argv[1];
  if(data->errorString[0]) {
    argv[0] = v8::Exception::Error(NanNew<v8::String>(data->errorString));
  } else {
    argv[0] = NanUndefined();
  }
  data->callback->Call(1, argv);

  delete data->callback;
  delete data;
  delete req;
}

NAN_METHOD(List) {
  NanScope();

  // callback
  if(!args[0]->IsFunction()) {
    NanThrowTypeError("First argument must be a function");
    NanReturnUndefined();
  }
  v8::Local<v8::Function> callback = args[0].As<v8::Function>();

  ListBaton* baton = new ListBaton();
  strcpy(baton->errorString, "");
  baton->callback = new NanCallback(callback);

  uv_work_t* req = new uv_work_t();
  req->data = baton;
  uv_queue_work(uv_default_loop(), req, EIO_List, (uv_after_work_cb)EIO_AfterList);

  NanReturnUndefined();
}

void EIO_AfterList(uv_work_t* req) {
  NanScope();
	
  ListBaton* data = static_cast<ListBaton*>(req->data);

  v8::Handle<v8::Value> argv[2];
  if(data->errorString[0]) {
    argv[0] = v8::Exception::Error(NanNew<v8::String>(data->errorString));
    argv[1] = NanUndefined();
  } else {
    v8::Local<v8::Array> results = NanNew<v8::Array>();
    int i = 0;
    for(std::list<ListResultItem*>::iterator it = data->results.begin(); it != data->results.end(); ++it, i++) {
      v8::Local<v8::Object> item = NanNew<v8::Object>();
      item->Set(NanNew<v8::String>("path"), NanNew<v8::String>((*it)->path.c_str()));
	  item->Set(NanNew<v8::String>("idn"), NanNew<v8::String>((*it)->idn.c_str()));
      results->Set(i, item);
    }
    argv[0] = NanUndefined();
    argv[1] = results;
  }
  data->callback->Call(2, argv);
   
  delete data->callback;
  for(std::list<ListResultItem*>::iterator it = data->results.begin(); it != data->results.end(); ++it) {
    delete *it;
  }
  delete data;
  delete req;
}

extern "C" {
  void init (v8::Handle<v8::Object> target)
  {
    NanScope();
    NODE_SET_METHOD(target, "open", Open);
	  NODE_SET_METHOD(target, "query", Query);
    NODE_SET_METHOD(target, "close", Close);
    NODE_SET_METHOD(target, "list", List);
  }
}

NODE_MODULE(visaport, init);

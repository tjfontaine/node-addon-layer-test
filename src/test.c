#include "stdlib.h"
#include "stdio.h"

#include "shim.h"

int test_func(shim_ctx_t* ctx, size_t argc, shim_val_t** argv)
{
  printf("we're in test_func, argc %zu\n", argc);

  int32_t i;
  uint32_t u;
  shim_jstring_t* S;
  shim_ConvertArguments(ctx, argc, argv, "iuS", &i, &u, &S);

  char *str = shim_EncodeString(ctx, S);
  printf("we have an argument of %d %u %s\n", i, u, str);
  free(str);

  SHIM_SET_RVAL(ctx, argv, shim_NumberValue(i));
  return TRUE;
}

int test_foo(shim_ctx_t* ctx, size_t argc, shim_val_t** argv)
{
  printf("we're in test_foo\n");

  shim_jstring_t* str;
  shim_ConvertArguments(ctx, argc, argv, "S", &str);

  char *cstr = shim_EncodeString(ctx, str);
  printf("we got %s\n", cstr);
  free(cstr);

  SHIM_SET_RVAL(ctx, argv, shim_NewStringCopyZ(ctx, "Goodbye Fool"));
  return TRUE;
}

int test_cb(shim_ctx_t* ctx, size_t argc, shim_val_t** argv)
{
  shim_val_t* fn = argv[0];
  shim_val_t* rval;

  shim_CallFunctionValue(ctx, NULL, *fn, 0, NULL, &rval);

  SHIM_SET_RVAL(ctx, argv, rval);
  return TRUE;
}

typedef struct cb_baton_s {
  shim_work_t req;
  shim_val_t* cb;
  int rval;
} cb_baton_t;

void cb_work(shim_work_t* req)
{
  cb_baton_t* baton = container_of(req, cb_baton_t, req);
  baton->rval = 42;
}

void cb_after(shim_ctx_t* ctx, shim_work_t* req, int status)
{
  cb_baton_t* baton = container_of(req, cb_baton_t, req);
  shim_val_t* argv[] = { shim_NumberValue(baton->rval) };
  shim_val_t* rval;
  shim_CallFunctionValue(ctx, NULL, *baton->cb, 1, argv, &rval);
  shim_RemoveValueRoot(ctx, baton->cb);
  free(baton);
}

int test_cb_async(shim_ctx_t* ctx, size_t argc, shim_val_t** argv)
{
  cb_baton_t* baton = malloc(sizeof(cb_baton_t));
  shim_val_t* fn = argv[0];
  shim_AddValueRoot(ctx, fn);
  baton->cb = fn;
  shim_queue_work(shim_default_loop(), &baton->req, cb_work, cb_after);
  return TRUE;
}

const char* WHATWHAT = "WHAT WHAT";

void weak_cb(shim_val_t* val, void* data)
{
  int32_t i;
  shim_ValueToECMAInt32(NULL, val, &i);
  printf("WeakCB %d %p %s\n", i, data, (const char*)data);
  shim_ValueDispose(val);
}

int test_weak(shim_ctx_t* ctx, size_t argc, shim_val_t** argv)
{
  shim_val_t* obj = argv[0];
  shim_ValueMakeWeak(ctx, obj, (void*)WHATWHAT, weak_cb);
  SHIM_SET_RVAL(ctx, argv, obj);
  return TRUE;
}

int initialize(shim_ctx_t* ctx, shim_val_t* exports, shim_val_t* module)
{
  shim_FunctionSpec funcs[] = {
    SHIM_FS_DEF(test_func, 0),
    SHIM_FS_DEF(test_foo, 0),
    SHIM_FS_DEF(test_cb, 0),
    SHIM_FS_DEF(test_cb_async, 0),
    SHIM_FS_DEF(test_weak, 0),
    SHIM_FS_END,
  };

  shim_DefineFunctions(ctx, exports, funcs);
  
  printf("we're initializing the c side\n");
  return TRUE;
}

SHIM_MODULE(addon_test, initialize)

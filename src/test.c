#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "shim.h"

int test_func(shim_ctx_t* ctx, shim_args_t* args)
{
  printf("we're in test_func, argc %zu\n", shim_args_length(args));

  int32_t i;
  uint32_t u;
  shim_val_t* S = malloc(sizeof(shim_val_t*));
  shim_unpack(ctx, args,
    SHIM_TYPE_INT32, &i,
    SHIM_TYPE_UINT32, &u,
    SHIM_TYPE_STRING, &S,
    SHIM_TYPE_UNKNOWN);

  const char *str = shim_string_value(S);
  printf("we have an argument of %d %u %s\n", i, u, str);

  shim_args_set_rval(ctx, args, shim_integer_new(ctx, i));
  return TRUE;
}

int test_foo(shim_ctx_t* ctx, shim_args_t* args)
{
  printf("we're in test_foo\n");

  shim_val_t* str = malloc(sizeof(shim_val_t*));
  shim_unpack(ctx, args, SHIM_TYPE_STRING, &str, SHIM_TYPE_UNKNOWN);

  const char *cstr = shim_string_value(str);
  printf("we got %s\n", cstr);

  shim_value_release(str);

  shim_args_set_rval(ctx, args, shim_string_new_copy(ctx, "Goodbye Fool"));
  return TRUE;
}

int test_cb(shim_ctx_t* ctx, shim_args_t* args)
{
  shim_val_t* fn = shim_args_get(args, 0);
  shim_val_t* rval = malloc(sizeof(shim_val_t*));

  shim_func_call_val(ctx, NULL, fn, 0, NULL, rval);

  shim_args_set_rval(ctx, args, rval);
  return TRUE;
}

typedef struct cb_baton_s {
  shim_val_t* cb;
  int rval;
} cb_baton_t;

void cb_work(shim_work_t* req, cb_baton_t* baton)
{
  printf("in cb_work\n");
  baton->rval = 42;
}

void cb_after(shim_ctx_t* ctx, shim_work_t* req, int status, cb_baton_t* baton)
{
  printf("in cb_after\n");
  shim_val_t* argv[] = { shim_number_new(ctx, baton->rval) };
  shim_val_t* rval = malloc(sizeof(shim_val_t*));
  shim_func_call_val(ctx, NULL, baton->cb, 1, argv, rval);
  shim_value_release(argv[0]);
  shim_value_release(rval);
  shim_persistent_dispose(baton->cb);
  free(baton);
}

int test_cb_async(shim_ctx_t* ctx, shim_args_t* args)
{
  printf("in cb_async\n");
  cb_baton_t* baton = malloc(sizeof(cb_baton_t));
  shim_val_t* fn = shim_persistent_new(ctx, shim_args_get(args, 0));
  printf("made persistent\n");
  baton->cb = fn;
  shim_queue_work((shim_work_cb)cb_work, (shim_after_work)cb_after, baton);
  printf("queued work\n");
  return TRUE;
}

const char* WHATWHAT = "WHAT WHAT";

void weak_cb(shim_val_t* val, void* data)
{
  int32_t i;
  shim_unpack_type(NULL, val, SHIM_TYPE_INT32, &i);
  printf("WeakCB %d %p %s\n", i, data, (const char*)data);
  shim_persistent_dispose(val);
}

int test_weak(shim_ctx_t* ctx, shim_args_t* args)
{
  shim_val_t* obj = shim_persistent_new(ctx, shim_args_get(args, 0));
  shim_obj_make_weak(ctx, obj, (void*)WHATWHAT, weak_cb);
  shim_args_set_rval(ctx, args, obj);
  return TRUE;
}

int test_str(shim_ctx_t* ctx, shim_args_t* args)
{
  shim_args_set_rval(ctx, args, shim_string_new_copy(ctx, "wtf"));
  return TRUE;
}

int test_pass_buff(shim_ctx_t* ctx, shim_args_t* args)
{
  shim_val_t* buf = shim_args_get(args, 0);
  size_t len = shim_buffer_length(buf);

  printf("test_pass_buff incoming buffer length %zu should be %zu",
    len, strlen("hello world"));

  char* data = shim_buffer_value(buf);

  shim_val_t* enc = shim_string_new_copyn(ctx, data, len);

  shim_args_set_rval(ctx, args, enc);

  return TRUE;
}

int initialize(shim_ctx_t* ctx, shim_val_t* exports, shim_val_t* module)
{
  shim_fspec_t funcs[] = {
    SHIM_FS(test_func),
    SHIM_FS(test_foo),
    SHIM_FS(test_cb),
    SHIM_FS(test_cb_async),
    SHIM_FS(test_weak),
    SHIM_FS(test_str),
    SHIM_FS(test_pass_buff),
    SHIM_FS_END,
  };

  shim_obj_set_funcs(ctx, exports, funcs);
  
  printf("we're initializing the c side\n");
  return TRUE;
}

SHIM_MODULE(addon_test, initialize)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "shim.h"

int test_func(shim_ctx_t* ctx, shim_args_t* args)
{
  //printf("we're in test_func, argc %zu\n", shim_args_length(args));

  int32_t i;
  uint32_t u;
  shim_val_t* S = shim_value_alloc();
  if(!shim_unpack(ctx, args,
    SHIM_TYPE_INT32, &i,
    SHIM_TYPE_UINT32, &u,
    SHIM_TYPE_STRING, &S,
    SHIM_TYPE_UNKNOWN))
    return FALSE;

  char *str = shim_string_value(S);
  //printf("we have an argument of %d %u %s -- %p\n", i, u, str, str);

  shim_value_release(S);
  free(str);

  shim_args_set_rval(ctx, args, shim_integer_new(ctx, i));
  return TRUE;
}

int test_foo(shim_ctx_t* ctx, shim_args_t* args)
{
  //printf("we're in test_foo\n");

  shim_val_t* str = shim_value_alloc();
  shim_unpack(ctx, args, SHIM_TYPE_STRING, &str, SHIM_TYPE_UNKNOWN);

  char *cstr = shim_string_value(str);
  //printf("we got %s\n", cstr);

  shim_value_release(str);
  free(cstr);

  shim_args_set_rval(ctx, args, shim_string_new_copy(ctx, "Goodbye Fool"));
  return TRUE;
}

int test_cb(shim_ctx_t* ctx, shim_args_t* args)
{
  shim_val_t* fn = shim_args_get(args, 0);
  shim_val_t* rval = shim_value_alloc();

  if(!shim_func_call_val(ctx, NULL, fn, 0, NULL, rval))
    return FALSE;

  shim_args_set_rval(ctx, args, rval);
  return TRUE;
}

typedef struct cb_baton_s {
  shim_persistent_t* cb;
  shim_persistent_t* obj;
  int rval;
} cb_baton_t;

void cb_work(shim_work_t* req, cb_baton_t* baton)
{
  //printf("in cb_work\n");
  baton->rval = 42;
}

void cb_after(shim_ctx_t* ctx, shim_work_t* req, int status, cb_baton_t* baton)
{
  //printf("in cb_after\n");
  shim_val_t* argv[] = { shim_number_new(ctx, baton->rval) };
  shim_val_t* rval = shim_value_alloc();

  shim_val_t* obj;
  shim_val_t* cb;

  if (baton->obj != NULL)
    shim_persistent_to_val(ctx, baton->obj, &obj);
  else
    obj = NULL;

  shim_persistent_to_val(ctx, baton->cb, &cb);

  shim_make_callback_val(ctx, obj, cb, 1, argv, rval);

  shim_value_release(argv[0]);
  shim_value_release(rval);

  shim_persistent_dispose(baton->cb);

  if (baton->obj != NULL)
    shim_persistent_dispose(baton->obj);

  free(baton);
}

int test_cb_async(shim_ctx_t* ctx, shim_args_t* args)
{
  //printf("in cb_async\n");
  cb_baton_t* baton = malloc(sizeof(cb_baton_t));
  shim_persistent_t* fn = shim_persistent_new(ctx, shim_args_get(args, 0));

  if (shim_args_length(args) > 1)
    baton->obj = shim_persistent_new(ctx, shim_args_get(args, 1));
  else
    baton->obj = NULL;

  //printf("made persistent\n");
  baton->cb = fn;
  shim_queue_work((shim_work_cb)cb_work, (shim_after_work)cb_after, baton);
  //printf("queued work\n");
  return TRUE;
}

const char* WHATWHAT = "WHAT WHAT";

void weak_cb(shim_persistent_t* pval, void* data)
{
  int32_t i;
  shim_val_t* val;
  shim_persistent_to_val(NULL, pval, &val);
  shim_unpack_type(NULL, val, SHIM_TYPE_INT32, &i);
  //printf("WeakCB %d %p %s\n", i, data, (const char*)data);
  shim_persistent_dispose(pval);
}

int test_weak(shim_ctx_t* ctx, shim_args_t* args)
{
  shim_val_t* val = shim_args_get(args, 0);
  shim_persistent_t* obj = shim_persistent_new(ctx, val);
  shim_obj_make_weak(ctx, obj, (void*)WHATWHAT, weak_cb);
  shim_args_set_rval(ctx, args, val);
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

  if (!shim_value_is(buf, SHIM_TYPE_BUFFER)) {
    shim_throw_error(ctx, "buf is not a buffer");
    return FALSE;
  }

  char* odata;

  if (!shim_unpack_type(ctx, buf, SHIM_TYPE_BUFFER, &odata))
    return FALSE;

  size_t len = shim_buffer_length(buf);

  //printf("test_pass_buff incoming buffer length %zu should be %zu",
  //  len, strlen("hello world"));

  char* data = shim_buffer_value(buf);

  if (odata != data) {
    shim_throw_error(ctx, "odata doesn't match data");
    return FALSE;
  }

  shim_val_t* enc = shim_string_new_copyn(ctx, data, len);

  shim_args_set_rval(ctx, args, enc);

  return TRUE;
}

int test_undefined(shim_ctx_t* ctx, shim_args_t* args)
{
  shim_args_set_rval(ctx, args, shim_undefined());
  return TRUE;
}

int test_null(shim_ctx_t* ctx, shim_args_t* args)
{
  shim_args_set_rval(ctx, args, shim_null());
  return TRUE;
}

int test_cb_null(shim_ctx_t* ctx, shim_args_t* args)
{
  shim_val_t* fn = shim_args_get(args, 0);
  shim_val_t* rval = shim_value_alloc();

  shim_val_t* argv[] = {
    shim_null(),
  };

  shim_func_call_val(ctx, NULL, fn, 1, argv, rval);

  shim_args_set_rval(ctx, args, rval);
  return TRUE;
}

int test_except(shim_ctx_t* ctx, shim_args_t* args)
{
  shim_throw_error(ctx, "Something went wrong");
  return FALSE;
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
    SHIM_FS(test_undefined),
    SHIM_FS(test_null),
    SHIM_FS(test_cb_null),
    SHIM_FS(test_except),
    SHIM_FS_END,
  };

  shim_obj_set_funcs(ctx, exports, funcs);
  
  //printf("we're initializing the c side\n");
  return TRUE;
}

SHIM_MODULE(addon_test, initialize)

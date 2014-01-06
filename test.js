if (process.execArgv.indexOf('--expose-gc') == -1) {
  function _gc() {
    console.error("please run with --expose-gc");
  }
} else {
  _gc = gc;
}

if (!/v0.8/.test(process.version)) {
  setInterval(_gc, 1).unref();
}

var tt = require('tt');
var b = require('bindings')('addon_test');

function test(name, cb) {
  return tt(name, function(t) {
    var _end = t.end;
    t.end = function() {
      _gc();
      _end();
    };
    cb(t);
    _gc();
  });
}

test('test_func', function(t) {
  t.strictEqual(40, b.test_func(40, 44, 'baz'));
  t.end();
});

test('test_func_args', function(t) {
  var threw = false;
  try {
    b.test_func('foo', 'bar', 123);
  } catch(e) {
    threw = true;
    t.strictEqual(e.message, "Argument 0 not of type SHIM_TYPE_INT32");
  } finally {
    t.strictEqual(threw, true);
    t.end();
  }
});

test('test_foo', function(t) {
  t.strictEqual('Goodbye Fool', b.test_foo('Hello World'));
  t.end();
});

test('test_cb', function(t) {
  t.strictEqual('barbarella', b.test_cb(function () {
    return "barbarella";
  }));
  t.end();
})

test('test_cb_async', function(t) {
  var tick = false;
  b.test_cb_async(function(foo) {
    t.strictEqual(tick, true);
    _gc();
    t.strictEqual(42, foo);
    _gc();
    t.end();
  });
  tick = true;
});

test('test_make_weak', function(t) {
  var a = {};
  a.weak = b.test_weak({a:42})

  setTimeout(function() {
    delete a.weak;
    a.weak = null;
    _gc();
    t.end();
  }, 0);
  _gc();
});

test('test_str', function(t) {
  t.strictEqual('wtf', b.test_str());
  t.end();
});

test('test_pass_buff', function(t) {
  var contents = 'hello world'
  var helloworld = new Buffer(contents);
  var ret = b.test_pass_buff(helloworld);
  t.strictEqual(ret, contents);
  t.throws(function() {
    b.test_pass_buff('buf');
  });
  t.end();
})

test('test_undefined', function(t) {
  t.strictEqual(b.test_undefined(), undefined);
  t.end();
});

test('test_null', function(t) {
  t.strictEqual(b.test_null(), null);
  t.end();
});

test('test_cb_null', function(t) {
  b.test_cb_null(function(arg) {
    t.strictEqual(arg, null);
    t.end();
  });
});

test('test_cb_except', function(t) {
  var threw = false;
  try {
    b.test_cb(function() {
      throw new Error("we errored");
    });
  } catch (e) {
    threw = true;
    t.strictEqual(e.message, 'we errored');
  } finally {
    t.strictEqual(threw, true);
    t.end();
  }
});

test('test_except', function(t) {
  var threw = false;
  try {
    b.test_except();
  } catch(e) {
    threw = true;
    t.strictEqual(e.message, 'Something went wrong');
  } finally {
    t.strictEqual(threw, true);
    t.end();
  }
});

test('test_cb_async_except', function(t) {
  var msg = "We Errored";
  var d = require('domain').create();
  d.on('error', function (err) {
    t.strictEqual(err.message, msg);
    t.end();
  });
  d.run(function() {
    var obj = { domain: d };
    b.test_cb_async(function() {
      throw new Error(msg);
    }, obj);
  })
});

test('test_external', function(t) {
  var handle = b.test_make_external();
  b.test_get_external(handle);
  t.end();
});

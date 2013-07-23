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

var b = require('bindings')('addon_test');

console.log(b);

console.log("test_func ret", b.test_func(40, 44, "baz"));
console.log("test_foo ret", b.test_foo("hello world"));

console.log("test_cb ret", b.test_cb(function () {
  _gc();
  console.log("in cb");
  return "barbarella";
}));

b.test_cb_async(function(foo) {
  _gc();
  console.log("asyncd", foo);
  _gc();
});

console.log("tick");

_gc();
_gc();

var a = {};
a.weak = b.test_weak({a:42})

_gc();
_gc();

setTimeout(function() {
  console.log('clearing out weak', a.weak);
  delete a.weak;
  a.weak = null;
  _gc();
}, 0);

console.log("test_str", b.test_str());

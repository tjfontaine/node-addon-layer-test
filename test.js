var b = require('bindings')('addon_test');
console.log(b);
console.log("test_func ret", b.test_func(40, 44, "baz"));
console.log("test_foo ret", b.test_foo("hello world"));
console.log("test_cb ret", b.test_cb(function () {
  gc();
  console.log("in cb");
  return "barbarella";
}));
b.test_cb_async(function(foo) {
  gc();
  console.log("asyncd", foo);
  gc();
});
console.log("tick");
gc();
gc();
var a = {};
a.weak = b.test_weak({a:42})
gc();
gc();
setTimeout(function() {
  console.log('clearing out weak', a.weak);
  delete a.weak;
  a.weak = null;
  gc();
}, 0);
setInterval(gc, 1).unref();

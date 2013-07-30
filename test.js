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


function test1() {
  console.log("calling test_func");
  console.log("test_func ret", b.test_func(40, 44, "baz"));
}

function test2() {
  console.log("test_foo ret", b.test_foo("hello world"));
}

function test3() {
  console.log("test_cb ret", b.test_cb(function () {
    _gc();
    console.log("in cb");
    return "barbarella";
  }));
}

function test4() {
  console.log("calling into test_async");

  b.test_cb_async(function(foo) {
    _gc();
    console.log("asyncd", foo);
    _gc();
  });

  console.log("tick");
}

function test5() {
  var a = {};
  a.weak = b.test_weak({a:42})

  setTimeout(function() {
    console.log('clearing out weak', a.weak);
    delete a.weak;
    a.weak = null;
    _gc();
  }, 0);
}

function test6() {
  console.log("test_str", b.test_str());
}

function test7() {
  var helloworld = new Buffer("hello world");
  console.log("test_pass_buff", b.test_pass_buff(helloworld));
}

setInterval(function () {
  test1();
  test2();
  test3();
  //test4();
  test5();
  test6();
  test7();
}, 1000);


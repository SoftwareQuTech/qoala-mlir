# LIR

```mlir
%q = init() : () -> f64
%qa = h(%q0) : (f64) -> f64
%t = recv() {remote = "client"} : () -> i32
%qb = rot(%qa, %t) : (i32, f64) -> f64
%m = meas(%qb) : (f64) -> i1
```

Split into netqasm functions:
(assuming there is a reason to *not* move all
quantum code in one netqasm function after the recv)

```mlir
qoala.netqasm @func1() -> f64 {
    %q = init() : () -> f64
    %qa = h(%q0) : (f64) -> f64
    return %q1
}

qoala.netqasm @func2(%t: i32, %q: f64) -> i1 {
    %qa = rot(%q, %t) : (i32, f64) -> f64
    %m = meas(%qa) : (f64) -> i1
    return %m
}

%q = call @func1() : () -> f64
%t = recv() {remote = "client"} : () -> i32
%m = call @func2(%t, %q) : (i32, f64) -> i1
```


Map qubits

```mlir
// func1: 1 logical qubit lq1: <q, qa>
// map lq1 to C
qoala.netqasm @func1() -> !qubit_c {
    %q = init() : () -> !qubit_c
    %qa = h(%q0) : (!qubit_c) -> !qubit_c
    return %q1
}

qoala.netqasm @func2(%t: i32, %q: f64) -> i1 {
    %qa = rot(%q, %t) : (i32, f64) -> f64
    %m = meas(%qa) : (f64) -> i1
    return %m
}

%q1 = call @func1() : () -> f64
%t = recv() {remote = "client"} : () -> i32
%m = call @func2(%t, %q1) : (i32, f64) -> i1

```

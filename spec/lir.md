# HIR

```mlir
%q = init() : () -> f64
%qa = h(%q0) : (f64) -> f64
%t = recv() {remote = "client"} : () -> i32
%qb = rot(%qa, %t) : (i32, f64) -> f64
%m = meas(%qb) : (f64) -> i1
```

# LIR

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

# LIR-M

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


# BQC example
### HIR

```mlir
%epr1 = entangle() : () -> !qubit
%epr2 = entangle() : () -> !qubit
%delta1 = recv() : () -> i32
%epr2b = rot(%epr2a, %delta1) : (!qubit, i32) -> !qubit
%m1 = meas(%epr2b) : (!qubit) -> i1
send(%m1) : (i1) -> ()
%delta2 = recv() : () -> i32
%epr1b = rot(%epr1a, %delta2) : (!qubit, i32) -> !qubit
%m2 = meas(%epr1b) : (!qubit) -> i1
```

### LIR

Split into netqasm functions:
(assuming there is a reason to *not* move all
quantum code in one netqasm function after the recv)

```mlir
qoala.netqasm @func1() -> (!qubit, !qubit) {
    %q1 = entangle() : () -> !qubit
    %q2 = entangle() : () -> !qubit
    return (%q1, %q2)
}

qoala.netqasm @func2(%delta: i32, %q1: !qubit, %q2: !qubit) -> (i1, !qubit) {
    %q2a = rot(%q2, %delta) : (!qubit, i32) -> !qubit
    %m = meas(%q2a) : (!qubit) -> i1
    return (%m, %q1)
}

qoala.netqasm @func3(%delta: i32, %q: !qubit) -> i1 {
    %qa = rot(%q, %delta) : (!qubit, i32) -> !qubit
    %m = meas(%qa) : (!qubit) -> i1
    return %m
}

%epr1, %epr2 = call @func1() : () -> (!qubit, !qubit)
%delta1 = recv() : () -> i32
%m1, %epr1 = call @func2(%delta1, %epr1, %epr2) -> (i1, !qubit)
send(%m1) : (i1) -> ()
%delta2 = recv() : () -> i32
%m1 = call @func3(%delta2, %epr1) -> i1
```

### LIR-M

Map qubits (using SMT?)

```mlir
qoala.netqasm @func1() -> (!qubit_m, !qubit_c) {
    %q1 = entangle_m() : () -> !qubit_m
    %q2 = entangle_c() : () -> !qubit_c
    return (%q1, %q2)
}

qoala.netqasm @func2(%delta: i32, %q1: !qubit_m, %q2: !qubit_c) -> (i1, !qubit_m) {
    %q2a = rot_c(%q2, %delta) : (!qubit_c, i32) -> !qubit_c
    %m = meas_c(%q2a) : (!qubit_c) -> i1
    return (%m, %q1)
}

qoala.netqasm @func3(%delta: i32, %q: !qubit_m) -> i1 {
    %qa = rot_m(%q, %delta) : (!qubit_m, i32) -> !qubit_m
    %m = meas_m(%qa) : (!qubit_m) -> i1
    return %m
}

%epr1, %epr2 = call @func1() : () -> (!qubit_m, !qubit_c)
%delta1 = recv() : () -> i32
%m1, %epr1 = call @func2(%delta1, %epr1, %epr2) -> (i1, !qubit_m)
send(%m1) : (i1) -> ()
%delta2 = recv() : () -> i32
%m1 = call @func3(%delta2, %epr1) -> i1
```

struct Value {
    data: u64,
    grad: u64,
    
    // graph construction
    _backward: String, // ??
    _prev: u64,// hashmap? hashset?
    _op: String,
}

fn value_constructor(data: u64, grad: u64) -> Value {
    Value {
        data,
        grad,
        _backward: String::from(""),
        _prev: 1,// hashmap? hashset?
        _op: String::from(""),
    }
}

fn trace(root: &Value) {
    // Find all nodes
}

fn main() {

    let mut x = value_constructor(1, 0);

    x.grad = 1;

    println!("data: {}, grad: {}", x.data, x.grad);
}

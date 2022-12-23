struct Node<T, U> {
    data: T,
    grad: U,
    
    // graph construction
    _backward: String, // ??
    _prev: u64,// hashmap? hashset?
    _op: String,
}

// Should this be a trait/impl?
fn print_type_of<T>(_: &T) {
    println!("{}", std::any::type_name::<T>())
}


// Should this be a trait/impl?
fn node_constructor<T, U>(data: T, grad: U) -> Node<T, U> {
    Node {
        data,
        grad,
        _backward: String::from(""),
        _prev: 1,// hashmap? hashset?
        _op: String::from(""),
    }
}

// Trace the graph
// fn trace(root: &Value) {
//     // Find all nodes
// }

fn main() {

    let mut x = node_constructor(1, 0.0);

    x.grad = 1.0;

    println!("data: {}, grad: {}", x.data, x.grad);
    print_type_of(&x.grad);
}

use std::ops::{Add, Sub, Mul, Div};

struct Node<T, U> {
    data: T,
    grad: U, // Should this always be f64?
    
    // graph construction
    _backward: String, // ??
    _prev: u64,// hashmap? hashset? vector?
    _op: String
}

impl<T, U> Node<T, U>{
    fn new(data: T, grad: U) -> Self {
        Self {
            data,
            grad,
            _backward: String::from(""),
            _prev: 1,// hashmap? hashset?
            _op: String::from(""),
        }
    }
}

// Add
impl<T: Add<Output = T>, U> Add for Node<T, U> {
    type Output = Self;

    fn add(self, other: Self) -> Self::Output {
        Self {
            data: self.data + other.data,
            grad: self.grad, // Can be propogated as result's grad

            _backward: String::from(""),
            _prev: 1,// hashmap? hashset?
            _op: String::from("+"),
        }
    }
}

// Sub
impl<T: Sub<Output = T>, U> Sub for Node<T, U> {
    type Output = Self;

    fn sub(self, other: Self) -> Self::Output {
        Self {
            data: self.data - other.data,
            grad: self.grad, // Can be propogated as result's grad

            _backward: String::from(""),
            _prev: 1,// hashmap? hashset?
            _op: String::from("+"),
        }
    }
}

// Mul
impl<T: Mul<Output = T>, U> Mul for Node<T, U> {
    type Output = Self;

    fn mul(self, other: Self) -> Self::Output {
        Self {
            data: self.data * other.data,
            grad: self.grad,

            _backward: String::from(""),
            _prev: 1,// hashmap? hashset?
            _op: String::from("+"),
        }
    }
}

// Div
impl<T: Div<Output = T>, U> Div for Node<T, U> {
    type Output = Self;

    fn div(self, other: Self) -> Self::Output {
        Self {
            data: self.data / other.data,
            grad: self.grad, // Can be propogated as result's grad

            _backward: String::from(""),
            _prev: 1,// hashmap? hashset?
            _op: String::from("+"),
        }
    }
}

// Should this be a trait/impl?
fn print_type<T>(_: &T) {
    println!("{}", std::any::type_name::<T>())
}

// Trace the graph
// fn trace(root: &Value) {
//     // Find all nodes
// }

fn main() {

    let x = Node::new(1.0, 0.0);
    let y = Node::new(2.0, 1.0);

    let a: Node<f64, f64>;

    a = x + y;

    println!("data: {}, grad: {}", a.data, a.grad);
    print_type(&a);
}

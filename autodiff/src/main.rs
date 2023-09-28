use std::collections::HashMap;
use std::ops::{Add, Sub, Mul, Div};

#[derive(Debug, Clone, Copy)]
enum Op {
    New,
    Add,
    Sub,
    Mul,
    Div,
}

#[derive(Clone)]
struct Node<T, U> {
    data: T,
    grad: U,
    op: Op,
    backward: String,
    prev: HashMap<usize, Node<T, U>>,
    label: String
}

impl<T, U> Node<T, U>
where
    T: Add<Output = T> + Sub<Output = T> + Mul<Output = T> + Div<Output = T> + Clone,
    U: Clone,
{
    fn new(data: T, grad: U, label: Option<String>) -> Self {
        Self {
            data,
            grad,
            op: Op::New,
            backward: String::from(""),
            prev: HashMap::new(),
            label: label.unwrap_or(String::from("")),
        }
    }
}


// Add
impl<T, U> Add for Node<T, U>
where
    T: Add<Output = T> + Copy,
    U: Copy,
{
    type Output = Self;

    fn add(self, other: Self) -> Self::Output {
        Self {
            data: self.data + other.data,
            grad: self.grad,
            op: Op::Add,
            backward: String::from(""),
            prev: {
                let mut map = HashMap::new();
                map.insert(0, self);
                map.insert(1, other);
                map
            },
            label: String::from("")
        }
    }
}

// Sub
impl<T, U> Sub for Node<T, U>
where
    T: Sub<Output = T> + Copy,
    U: Copy,
{
    type Output = Self;

    fn sub(self, other: Self) -> Self::Output {
        Self {
            data: self.data - other.data,
            grad: self.grad,
            op: Op::Sub,
            backward: String::from(""),
            prev: {
                let mut map = HashMap::new();
                map.insert(0, self);
                map.insert(1, other);
                map
            },
            label: String::from("")
        }
    }
}

// Mul
impl<T, U> Mul for Node<T, U>
where
    T: Mul<Output = T> + Copy,
    U: Copy,
{
    type Output = Self;

    fn mul(self, other: Self) -> Self::Output {
        Self {
            data: self.data * other.data,
            grad: self.grad,
            op: Op::Mul,
            backward: String::from(""),
            prev: {
                let mut map = HashMap::new();
                map.insert(0, self);
                map.insert(1, other);
                map
            },
            label: String::from("")
        }
    }
}

// Div
impl<T, U> Div for Node<T, U>
where
    T: Div<Output = T> + Copy,
    U: Copy,
{
    type Output = Self;

    fn div(self, other: Self) -> Self::Output {
        Self {
            data: self.data / other.data,
            grad: self.grad,
            op: Op::Div,
            backward: String::from(""),
            prev: {
                let mut map = HashMap::new();
                map.insert(0, self);
                map.insert(1, other);
                map
            },
            label: String::from("")
        }
    }
}

use std::fmt;

impl fmt::Display for Op {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Op::New => write!(f, "new node"),
            Op::Add => write!(f, "+"),
            Op::Sub => write!(f, "-"),
            Op::Mul => write!(f, "*"),
            Op::Div => write!(f, "/"),
        }
    }
}


fn main() {

    // Create nodes (data)
    let w = Node::new(21241242.0, 0.0, Some(String::from("first node")));
    let x = Node::new(32414214.0, 0.0, Some(String::from("second node")));
    let y = Node::new(42412412.0, 1.0, None);
    let z = Node::new(52141241.0, 1.0, None);

    let mut a = x * y;

    a.label = String::from("LABEL A");
    
    let mut b = w + z;

    b.label = String::from("LABEL B");
    
    println!("\n");

    println!("label: {}, data: {}, grad: {}, backward: {}, prev_0: {},  prev_1: {}, op: {}\n", a.label, a.data, a.grad, a.backward, a.prev[&0].data, a.prev[&1].data, a.op);
    println!("label: {}, data: {}, grad: {}, backward: {}, prev_0: {},  prev_1: {}, op: {}\n", b.label, b.data, b.grad, b.backward, b.prev[&0].data, b.prev[&1].data, b.op);
}
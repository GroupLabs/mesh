use std::collections::HashMap;
use std::ops::{Add, Sub, Mul, Div};
use std::rc::Rc;

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
    id: usize,
    data: T,
    grad: U,
    op: Op,
    backward: String,
    prev: HashMap<usize, Rc<Node<T, U>>>,
}

fn generate_id() -> usize {
    use std::sync::atomic::{AtomicUsize, Ordering};
    static COUNTER: AtomicUsize = AtomicUsize::new(0);
    COUNTER.fetch_add(1, Ordering::SeqCst)
}


impl<T, U> Node<T, U>
where
    T: Add<Output = T> + Sub<Output = T> + Mul<Output = T> + Div<Output = T> + Clone,
    U: Clone,
{
    fn new(data: T, grad: U, label: Option<String>) -> Self {
        let id = generate_id();
        Self {
            id,
            data,
            grad,
            op: Op::New,
            backward: String::from(""),
            prev: HashMap::new(),
            label: label.unwrap_or(String::from("")),
        }
    }
}

fn merge_prev<T, U>(lhs: &Node<T, U>, rhs: &Node<T, U>) -> HashMap<usize, Rc<Node<T, U>>>
where
    T: Clone,
    U: Clone,
{
    let mut map = lhs.prev.clone();
    for (k, v) in rhs.prev.iter() {
        map.entry(*k).or_insert(v.clone());
    }
    map
}

// Add
impl<T, U> Add for Node<T, U>
where
    T: Add<Output = T> + Copy,
    U: Copy,
{
    type Output = Self;

    fn add(self, other: Self) -> Self::Output {
        let prev = merge_prev(&self, &other);
        prev.insert(self.id, Rc::new(self.clone()));
        prev.insert(other.id, Rc::new(other.clone()));

        Self {
            id: Self::generate_id(),
            data: self.data + other.data,
            grad: self.grad,
            op: Op::Add,
            backward: String::from(""),
            prev,
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
        let prev = merge_prev(&self, &other);
        prev.insert(self.id, Rc::new(self.clone()));
        prev.insert(other.id, Rc::new(other.clone()));

        Self {
            id: Self::generate_id(),
            data: self.data - other.data,
            grad: self.grad,
            op: Op::Sub,
            backward: String::from(""),
            prev,
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
        let prev = merge_prev(&self, &other);
        prev.insert(self.id, Rc::new(self.clone()));
        prev.insert(other.id, Rc::new(other.clone()));

        Self {
            id: Self::generate_id(),
            data: self.data * other.data,
            grad: self.grad,
            op: Op::Mul,
            backward: String::from(""),
            prev,
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
            id: Self::generate_id(),
            data: self.data /
            other.data,
            grad: self.grad,
            op: Op::Div,
            backward: String::from(""),
            prev: {
                let mut map = HashMap::new();
                map.insert(0, Rc::new(self));
                map.insert(1, Rc::new(other));
                map
            },
        }
    }
}

use std::fmt;

impl fmt::Display for Op {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
        Op::New => write!(f, "NEW"),
        Op::Add => write!(f, "+"),
        Op::Sub => write!(f, "-"),
        Op::Mul => write!(f, "*"),
        Op::Div => write!(f, "/"),
        }
    }
}

impl<T, U> fmt::Display for Node<T, U>
where
T: fmt::Display + Clone,
U: fmt::Display + Clone,
{
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
        f,
        "Node {{ data: {}, grad: {}, op: {}, backward: {}, prev: {} }}",
        self.data,
        self.grad,
        self.op,
        self.backward,
        format_prev(&self.prev)
        )
    }
}

fn format_prev<T, U>(prev: &HashMap<usize, Rc<Node<T, U>>>) -> String
where
T: fmt::Display + Clone,
U: fmt::Display + Clone,
{
    let entries: Vec<String> = prev
    .iter()
    .map(|(k, v)| format!("{}: Node {{ data: {}, grad: {}, op: {} }}", k, v.data, v.grad, v.op))
    .collect();
    format!("{{ {} }}", entries.join(", "))
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
    // let x = Node::new(3.0, 0.0);
    // let y = Node::new(4.0, 1.0);
    // let a = Node::new(4.0, 1.0);

    // let b = a + x;
    // let c = b + y;

    println!("c: {}", c);
}
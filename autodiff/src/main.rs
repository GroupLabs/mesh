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
    label: String,
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

// Add
impl<T, U> Add for Node<T, U>
where
    T: Add<Output = T> + Copy,
    U: Copy,
{
    type Output = Self;

    fn add(self, other: Self) -> Self::Output {
        let mut prev = HashMap::new();  // Start with an empty HashMap
        prev.insert(self.id, Rc::new(self.clone()));  // Insert only the direct parent nodes
        prev.insert(other.id, Rc::new(other.clone()));

        Self {
            id: generate_id(),
            data: self.data + other.data,
            grad: self.grad,  // You might want to consider how to handle grad for the new node
            op: Op::Add,
            backward: String::from(""),
            prev,  // Using the populated `prev` HashMap
            label: String::from(""),
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
        let mut prev = HashMap::new();  // Start with an empty HashMap
        prev.insert(self.id, Rc::new(self.clone()));  // Insert only the direct parent nodes
        prev.insert(other.id, Rc::new(other.clone()));

        Self {
            id: generate_id(),
            data: self.data - other.data,
            grad: self.grad,
            op: Op::Sub,
            backward: String::from(""),
            prev,
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
        let mut prev = HashMap::new();  // Start with an empty HashMap
        prev.insert(self.id, Rc::new(self.clone()));  // Insert only the direct parent nodes
        prev.insert(other.id, Rc::new(other.clone()));

        Self {
            id: generate_id(),
            data: self.data * other.data,
            grad: self.grad,
            op: Op::Mul,
            backward: String::from(""),
            prev,
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
        let mut prev = HashMap::new();  // Start with an empty HashMap
        prev.insert(self.id, Rc::new(self.clone()));  // Insert only the direct parent nodes
        prev.insert(other.id, Rc::new(other.clone()));

        Self {
            id: generate_id(),
            data: self.data /
            other.data,
            grad: self.grad,
            op: Op::Div,
            backward: String::from(""),
            prev,
            label: String::from("")
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
            "id: {}, label: {}, data: {}, grad: {}, backward: {}, op: {}, prev: {}",
            self.id,
            self.label,
            self.data,
            self.grad,
            self.backward,
            self.op,
            format_prev(&self.prev)  // Use format_prev to get the string representation of prev
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
        .map(|(k, v)| format!("{}: {}", k, v.as_ref()))  // Remove the label to see the full graph
        .collect();
    format!("{{ {} }}", entries.join(", "))
}

fn main() {
    
    let _none_example = Node::new(23.0, 1.0, None); // Unlabelled nodes are allowed
    
    let v = Node::new(23.0, 1.0, Some(String::from("V"))); // Unlabelled nodes are allowed

    // Create nodes (data)
    let w = Node::new(3.0, 0.0, Some(String::from("W")));
    let x = Node::new(4.0, 0.0, Some(String::from("X")));
    let y = Node::new(5.0, 1.0, Some(String::from("Y")));
    let z = Node::new(6.0, 1.0, Some(String::from("Z")));
    
    
    let mut a = v * w;
    a.label = String::from("A");
    
    let mut b = y + z;
    
    b.label = String::from("B");
    
    let mut c = a + b + x;
    
    c.label = String::from("C");
    
    println!("\n");
    
    println!("{}\n", c);
}
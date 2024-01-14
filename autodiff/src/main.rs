use std::collections::HashMap;
use std::ops::{Add, Sub, Mul, Div, AddAssign};
use std::rc::Rc;
use std::cell::RefCell;

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
    prev: HashMap<usize, Rc<RefCell<Node<T, U>>>>,
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
    type Output = Node<T, U>;

    fn add(self, other: Self) -> Self::Output {
        let mut prev = HashMap::new();
        prev.insert(self.id, Rc::new(RefCell::new(self.clone())));
        prev.insert(other.id, Rc::new(RefCell::new(other.clone())));

        Node {
            id: generate_id(),
            data: self.data + other.data,
            grad: self.grad,
            op: Op::Add,
            backward: String::from(""),
            prev,
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
    type Output = Node<T, U>;

    fn sub(self, other: Self) -> Self::Output {
        let mut prev = HashMap::new();
        prev.insert(self.id, Rc::new(RefCell::new(self.clone())));
        prev.insert(other.id, Rc::new(RefCell::new(other.clone())));

        Node {
            id: generate_id(),
            data: self.data - other.data,
            grad: self.grad,
            op: Op::Sub,
            backward: String::from(""),
            prev,
            label: String::from(""),
        }
    }
}

// Mul
impl<T, U> Mul for Node<T, U>
where
    T: Mul<Output = T> + Copy,
    U: Copy,
{
    type Output = Node<T, U>;

    fn mul(self, other: Self) -> Self::Output {
        let mut prev = HashMap::new();
        prev.insert(self.id, Rc::new(RefCell::new(self.clone())));
        prev.insert(other.id, Rc::new(RefCell::new(other.clone())));

        Node {
            id: generate_id(),
            data: self.data * other.data,
            grad: self.grad,
            op: Op::Mul,
            backward: String::from(""),
            prev,
            label: String::from(""),
        }
    }
}

// Div
impl<T, U> Div for Node<T, U>
where
    T: Div<Output = T> + Copy,
    U: Copy,
{
    type Output = Node<T, U>;

    fn div(self, other: Self) -> Self::Output {
        let mut prev = HashMap::new();
        prev.insert(self.id, Rc::new(RefCell::new(self.clone())));
        prev.insert(other.id, Rc::new(RefCell::new(other.clone())));

        Node {
            id: generate_id(),
            data: self.data / other.data,
            grad: self.grad,
            op: Op::Div,
            backward: String::from(""),
            prev,
            label: String::from(""),
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
            format_prev(&self.prev)
        )
    }
}

trait FromF64: Sized {
    fn from_f64(n: f64) -> Self;
}

impl FromF64 for f32 {
    fn from_f64(n: f64) -> Self {
        n as f32
    }
}

impl FromF64 for f64 {
    fn from_f64(n: f64) -> Self {
        n
    }
}

trait ToF64 {
    fn to_f64(self) -> f64;
}

impl ToF64 for f32 {
    fn to_f64(self) -> f64 {
        self as f64
    }
}

impl ToF64 for f64 {
    fn to_f64(self) -> f64 {
        self
    }
}

impl<T, U> Node<T, U>
where
    T: Copy + Clone + Add<Output = T> + Sub<Output = T> + Mul<Output = T> + Div<Output = T> + ToF64,
    U: Copy + Clone + AddAssign + FromF64 + Mul<Output = U> + Add<Output = U>,
{
    fn derivative(&self, input: &Self) -> U {
        match self.op {
            Op::Add => U::from_f64(1.0),
            Op::Sub => {
                if self.id == input.id {
                    U::from_f64(1.0)
                } else {
                    U::from_f64(-1.0)
                }
            }
            Op::Mul => {
                let other_id = self.prev.keys().find(|&&k| k != input.id).unwrap();
                let other_value = self.prev.get(&other_id).unwrap().borrow().data.clone();
                U::from_f64(other_value.to_f64())
            }
            Op::Div => {
                if self.id == input.id {
                    U::from_f64(1.0 / input.data.clone().to_f64())
                } else {
                    U::from_f64(-self.data.clone().to_f64() / (input.data.clone().to_f64() * input.data.clone().to_f64()))
                }
            }
            _ => U::from_f64(0.0),
        }
    }    
    
    fn backward(self, grad_output: U) -> Node<T, U> {
        let new_grad = self.grad + grad_output;
        let new_prev = self.prev.iter()
            .map(|(&k, v)| {
                let input = v.borrow().clone();
                let grad_input = self.derivative(&input) * grad_output;
                (k, Rc::new(RefCell::new(input.backward(grad_input))))
            })
            .collect::<HashMap<_, _>>();

        Node {
            grad: new_grad,
            prev: new_prev,
            ..self
        }
    }
}

fn format_prev<T, U>(prev: &HashMap<usize, Rc<RefCell<Node<T, U>>>>) -> String
where
    T: fmt::Display + Clone,
    U: fmt::Display + Clone,
{
    let entries: Vec<String> = prev
        .iter()
        .map(|(k, v)| format!("{}: {}", k, v.borrow()))
        .collect();
    format!("{{ {} }}", entries.join(", "))
}

fn main() {
    let x = Node::new(4.0, 0.0, Some(String::from("X")));
    let y = Node::new(5.0, 0.0, Some(String::from("Y")));

    let c = x * y;
    let new_c = c.backward(1.0);

    println!("Graph: {}", new_c);
}
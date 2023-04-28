// use std::collections::HashMap;
// use std::ops::{Add, Sub, Mul, Div};

// #[derive(Debug, Clone, Copy)]
// enum Op {
//     Add,
//     Sub,
//     Mul,
//     Div,
// }

// struct Node<T, U> {
//     data: T,
//     grad: U,
//     op: Op,
//     backward: String,
//     prev: HashMap<usize, Op>,
// }

// impl<T, U> Node<T, U>
// where
//     T: Add<Output = T> + Sub<Output = T> + Mul<Output = T> + Div<Output = T> + Copy,
//     U: Copy,
// {
//     fn new(data: T, grad: U) -> Self {
//         Self {
//             data,
//             grad,
//             op: Op::Add,
//             backward: String::from(""),
//             prev: HashMap::new(),
//         }
//     }
// }

// // Add
// impl<T, U> Add for Node<T, U>
// where
//     T: Add<Output = T> + Copy,
//     U: Copy,
// {
//     type Output = Self;

//     fn add(self, other: Self) -> Self::Output {
//         Self {
//             data: self.data + other.data,
//             grad: self.grad,
//             op: Op::Add,
//             backward: String::from(""),
//             prev: {
//                 let mut map = HashMap::new();
//                 map.insert(0, self.op);
//                 map.insert(1, other.op);
//                 map
//             },
//         }
//     }
// }

// // Sub
// impl<T, U> Sub for Node<T, U>
// where
//     T: Sub<Output = T> + Copy,
//     U: Copy,
// {
//     type Output = Self;

//     fn sub(self, other: Self) -> Self::Output {
//         Self {
//             data: self.data - other.data,
//             grad: self.grad,
//             op: Op::Sub,
//             backward: String::from(""),
//             prev: {
//                 let mut map = HashMap::new();
//                 map.insert(0, self.op);
//                 map.insert(1, other.op);
//                 map
//             },
//         }
//     }
// }

// // Mul
// impl<T, U> Mul for Node<T, U>
// where
//     T: Mul<Output = T> + Copy,
//     U: Copy,
// {
//     type Output = Self;

//     fn mul(self, other: Self) -> Self::Output {
//         Self {
//             data: self.data * other.data,
//             grad: self.grad,
//             op: Op::Mul,
//             backward: String::from(""),
//             prev: {
//                 let mut map = HashMap::new();
//                 map.insert(0, self.op);
//                 map.insert(1, other.op);
//                 map
//             },
//         }
//     }
// }

// // Div
// impl<T, U> Div for Node<T, U>
// where
//     T: Div<Output = T> + Copy,
//     U: Copy,
// {
//     type Output = Self;

//     fn div(self, other: Self) -> Self::Output {
//         Self {
//             data: self.data / other.data,
//             grad: self.grad,
//             op: Op::Div,
//             backward: String::from(""),
//             prev: {
//                 let mut map = HashMap::new();
//                 map.insert(0, self.op);
//                 map.insert(1, other.op);
//                 map
//             },
//         }
//     }
// }

// use std::fmt;

// impl fmt::Display for Op {
//     fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
//         match self {
//             Op::Add => write!(f, "+"),
//             Op::Sub => write!(f, "-"),
//             Op::Mul => write!(f, "*"),
//             Op::Div => write!(f, "/"),
//         }
//     }
// }

// fn main() {

//     let x = Node::new(1.0, 0.0);
//     let y = Node::new(2.0, 1.0);
//     let z = Node::new(2.0, 1.0);

//     let a = x + y;

//     a = a * z;

//     let b = a + x;

    
//     println!("data: {}, grad: {}, backward: {}, prev: {}", a.data, a.grad, a.backward, a.prev[&0]);
// }






use std::io;

fn main() {
    let a = [1, 2, 3, 4, 5];

    println!("Please enter an array index.");

    let mut index = String::new();

    io::stdin()
        .read_line(&mut index)
        .expect("Failed to read line");

    let index: usize = index
        .trim()
        .parse()
        .expect("Index entered was not a number");

    let element = a[index];

    println!("The value of the element at index {index} is: {element}");
}



































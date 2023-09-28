```
CODE
---
fn main() {
    
    let _none_example = Node::new(23.0, 1.0, None); // Unlabelled nodes are allowed
    
    let v = Node::new(23.0, 1.0, Some(String::from("V"))); // Unlabelled nodes are allowed

    // Create nodes (data)
    let w = Node::new(3.0, 0.0, Some(String::from("W")));
    let x = Node::new(4.0, 0.0, Some(String::from("X")));
    let y = Node::new(5.0, 1.0, Some(String::from("Y")));
    let z = Node::new(6.0, 1.0, Some(String::from("Z")));
    
    
    let mut a = v.clone() * w.clone();
    a.label = String::from("A");
    
    let mut b = y.clone() + z.clone();
    
    b.label = String::from("B");
    
    let mut c = a.clone() + b.clone() + x.clone();
    
    c.label = String::from("C");
    
    println!("\n");
    
    println!("{}\n", a);
    println!("{}\n", b);
    println!("{}\n", c);
    
    println!("{}\n", w);
    println!("{}\n", y);
    println!("{}\n", x);
    println!("{}\n", z);

}
---

OUTPUT
---
id: 5, label: A, data: 69, grad: 1, backward: , op: *, prev: 
  { 
    1: id: 1, label: W, data: 3, grad: 0, backward: , op: NEW, prev: {  }, 
    0: id: 0, label: , data: 23, grad: 1, backward: , op: NEW, prev: {  } 
  }

id: 6, label: B, data: 11, grad: 1, backward: , op: +, prev: 
  { 
    4: id: 4, label: Z, data: 6, grad: 1, backward: , op: NEW, prev: {  }, 
    3: id: 3, label: Y, data: 5, grad: 1, backward: , op: NEW, prev: {  } 
  }

# Since C = A + B + X 
# = (V*W) + (Y+Z) + X
# These are the most elementary pieces
# in the following example [(V*W) + (Y+Z)] becomes an unlabeled node
id: 9, label: C, data: 84, grad: 1, backward: , op: +, prev: 
  { 
    8: id: 8, label: , data: 80, grad: 1, backward: , op: +, prev: 
      { 
        7: id: 7, label: B, data: 11, grad: 1, backward: , op: +, prev: 
          { 
            5: id: 5, label: Z, data: 6, grad: 1, backward: , op: NEW, prev: {  }, 
            4: id: 4, label: Y, data: 5, grad: 1, backward: , op: NEW, prev: {  } 
          }, 
        6: id: 6, label: A, data: 69, grad: 1, backward: , op: *, prev: 
          { 
            1: id: 1, label: V, data: 23, grad: 1, backward: , op: NEW, prev: {  }, 
            2: id: 2, label: W, data: 3, grad: 0, backward: , op: NEW, prev: {  } 
          } 
      }, 
    3: id: 3, label: X, data: 4, grad: 0, backward: , op: NEW, prev: {  } 
  }

id: 1, label: W, data: 3, grad: 0, backward: , op: NEW, prev: {  }

id: 3, label: Y, data: 5, grad: 1, backward: , op: NEW, prev: {  }

id: 2, label: X, data: 4, grad: 0, backward: , op: NEW, prev: {  }

id: 4, label: Z, data: 6, grad: 1, backward: , op: NEW, prev: {  }
---
```

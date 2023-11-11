#[derive(Debug)]
#[repr(C)]
struct A {
    value_0: u64,
    value_1: u32,
    message: [u8; 24],
}

#[derive(Debug)]
#[repr(C)]
struct B {
    value_0: u32,
    message: [u8; 48],
}

fn main() {
    todo!()
}

use std::{io::Write, net::TcpStream};

#[derive(Debug)]
#[repr(C)]
struct A {
    value0: u64,
    value1: u32,
    message: [u8; 24],
}

#[derive(Debug)]
#[repr(C)]
struct B {
    value0: u32,
    message: [u8; 48],
}

unsafe fn any_as_u8_slice<T: Sized>(p: &T) -> &[u8] {
    ::core::slice::from_raw_parts((p as *const T).cast::<u8>(), ::core::mem::size_of::<T>())
}

fn main() -> std::io::Result<()> {
    let mut message_a: [u8; 24] = [0; 24];
    let mut message_a_writer: &mut [u8] = &mut message_a;
    message_a_writer.write_all("123 hello world from a!".as_bytes())?;
    let a = A {
        value0: 0xCAFE_CAFE,
        value1: 0xDEAD_BEEF,
        message: message_a,
    };
    let mut message_b: [u8; 48] = [0; 48];
    let mut message_b_writer: &mut [u8] = &mut message_b;
    message_b_writer.write_all("321 321 321 321 hello world from b! 321 321 321".as_bytes())?;
    let b = B {
        value0: 0x1805_2004,
        message: message_b,
    };

    let mut stream = TcpStream::connect("127.0.0.1:8080")?;
    stream.write_all(&[1])?;
    stream.write_all(unsafe { any_as_u8_slice(&a) })?;
    stream.write_all(&[2])?;
    stream.write_all(unsafe { any_as_u8_slice(&b) })?;
    println!("sent");
    Ok(())
}

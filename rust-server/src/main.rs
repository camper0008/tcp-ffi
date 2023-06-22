use std::{
    io::Read,
    mem::{size_of, transmute},
    net::{TcpListener, TcpStream},
};

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

fn next_variant<'a, I: Iterator<Item = &'a u8>>(iter: &mut I) -> Option<(u8, usize)> {
    loop {
        let variant = iter.next();

        match variant {
            Some(1) => break Some((1, size_of::<A>())),
            Some(2) => break Some((2, size_of::<B>())),
            Some(0) => continue,
            Some(_) => unreachable!("should parse structs fully each time"),
            None => break None,
        };
    }
}

fn handle_client(mut stream: TcpStream) -> std::io::Result<()> {
    // very small buffer to demonstrate that buffer size doesn't matter
    let mut io_buffer = [0u8; 32];
    let mut a_buffer = [0u8; size_of::<A>()];
    let mut b_buffer = [0u8; size_of::<B>()];
    let mut struct_buffer_idx = 0;
    let mut size_remaining = 0;
    let mut variant: u8 = 0;
    loop {
        io_buffer.iter_mut().for_each(|byte| *byte = 0);
        let io_buffer_length = stream.read(&mut io_buffer)?;
        if io_buffer_length == 0 {
            break Ok(());
        };
        let mut io_buffer_iter = io_buffer.iter();
        'parse: loop {
            if size_remaining == 0 {
                match next_variant(&mut io_buffer_iter) {
                    Some((next_variant, next_size_remaining)) => {
                        variant = next_variant;
                        size_remaining = next_size_remaining;
                        continue;
                    }
                    None => break 'parse,
                };
            }

            let Some(&next) = io_buffer_iter.next() else {
                break 'parse;
            };

            match variant {
                1 => a_buffer[struct_buffer_idx] = next,
                2 => b_buffer[struct_buffer_idx] = next,
                variant => unreachable!("unknown variant {variant}"),
            }

            size_remaining -= 1;
            struct_buffer_idx += 1;
            if size_remaining > 0 {
                continue;
            }

            struct_buffer_idx = 0;
            match variant {
                1 => unsafe {
                    let a = transmute::<[u8; size_of::<A>()], A>(a_buffer);
                    println!(
                        "recieved A {{\n    value_0={},\n    value_1={},\n    message='{}',\n}}",
                        a.value_0,
                        a.value_1,
                        a.message.iter().map(|c| *c as char).collect::<String>()
                    );
                },
                2 => unsafe {
                    let b = transmute::<[u8; size_of::<B>()], B>(b_buffer);
                    println!(
                        "recieved B {{\n    value_0={},\n    message='{}',\n}}",
                        b.value_0,
                        b.message.iter().map(|c| *c as char).collect::<String>()
                    );
                },
                variant => unreachable!("unknown variant {variant}"),
            }
        }
    }
}

fn main() -> std::io::Result<()> {
    let listener = TcpListener::bind("127.0.0.1:8080")?;

    for stream in listener.incoming() {
        handle_client(stream?)?;
    }
    Ok(())
}

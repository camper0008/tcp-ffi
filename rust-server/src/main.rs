#![allow(clippy::unused_io_amount)]

use std::{
    io::Read,
    mem::size_of,
    net::{TcpListener, TcpStream},
};

#[derive(Debug)]
#[repr(C)]
struct A {
    a: u64,
    b: u32,
    c: [u8; 24],
}

#[derive(Debug)]
#[repr(C)]
struct B {
    a: u32,
    b: [u8; 48],
}

fn handle_client(mut stream: TcpStream) -> std::io::Result<()> {
    let mut io_buffer = [0u8; 128];
    let mut a_buffer = [0u8; size_of::<A>()];
    let mut b_buffer = [0u8; size_of::<B>()];
    let mut struct_buffer_idx = 0;
    let mut size_remaining = 0;
    let mut variant: Option<u8> = None;
    loop {
        io_buffer.iter_mut().for_each(|byte| *byte = 0);
        let io_buffer_length = stream.read(&mut io_buffer)?;
        if io_buffer_length == 0 {
            break Ok(());
        };
        let mut iter = io_buffer.iter();
        'parse: loop {
            if size_remaining == 0 {
                match variant {
                    Some(variant) => match variant {
                        1 => size_remaining = size_of::<A>(),
                        2 => size_remaining = size_of::<B>(),
                        variant => panic!("unknown variant {variant}"),
                    },
                    None => 'variant_loop: loop {
                        variant = iter.next().copied();

                        match variant {
                            Some(1 | 2) => break 'variant_loop,
                            Some(0) => continue,
                            Some(_) => unreachable!("should parse fully"),
                            None => break 'parse,
                        }
                    },
                }
                continue;
            }

            let next = iter.next();

            if next.is_none() {
                break 'parse;
            }
            match variant {
                Some(1) => a_buffer[struct_buffer_idx] = *next.unwrap(),
                Some(2) => b_buffer[struct_buffer_idx] = *next.unwrap(),
                Some(variant) => panic!("unknown variant {variant}"),
                None => unreachable!(),
            }

            size_remaining -= 1;
            struct_buffer_idx += 1;
            if size_remaining > 0 {
                continue;
            }
            unsafe {
                match variant.unwrap() {
                    1 => {
                        let a = std::mem::transmute::<[u8; size_of::<A>()], A>(a_buffer);
                        println!(
                            "recieved A: a = {}, b = {}, c = '{}'",
                            a.a,
                            a.b,
                            a.c.iter().map(|c| *c as char).collect::<String>()
                        );
                    }
                    2 => {
                        let b = std::mem::transmute::<[u8; size_of::<B>()], B>(b_buffer);
                        println!(
                            "recieved B: a = {}, b = '{}'",
                            b.a,
                            b.b.iter().map(|c| *c as char).collect::<String>()
                        );
                    }
                    _ => unreachable!(),
                }
            }
            struct_buffer_idx = 0;
            variant = None;
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

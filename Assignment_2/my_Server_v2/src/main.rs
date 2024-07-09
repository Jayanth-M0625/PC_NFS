#[allow(unused_variables)]

use my_Server_v2::server_deps::*;
use std::net::*;
use std::io::{BufRead, Read, Write};
use std::fs;

fn main() {
    
    let listener = TcpListener::bind("127.0.0.1:7848").unwrap();
    loop {
        let stream = listener.accept();
        match stream {
            Ok((mut stream, addr)) => {
                println!("incoming connection from: {}", addr.to_string());
                handle_connection(stream);
            }
            Err(e) => { println!("connection failed") }
        }
    }
}
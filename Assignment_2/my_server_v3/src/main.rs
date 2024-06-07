#[allow(unused_variables)]

use my_server_v3::server_deps::*;
use std::net::*;

fn main() {
    
    let listener = TcpListener::bind("127.0.0.1:7848").unwrap();
    loop {
        let stream = listener.accept();
        match stream {
            Ok((stream, addr)) => {
                println!("incoming connection from: {}", addr.to_string());
                handle_connection(stream);
            }
            Err(_e) => { println!("connection failed") }
        }
    }
}
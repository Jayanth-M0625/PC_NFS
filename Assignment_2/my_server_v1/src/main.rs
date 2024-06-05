#[allow(unused_variables)]
use std::net::*;
use std::io::{BufRead, Read, Write};
// use std::fmt::Display;

fn handle_connection(mut stream: TcpStream) {
    println!("Connection established!");
    // let mut buffer = [0; 1024];
    // let bytes_read = stream.read(&mut buffer); //Raw 0s and 1s
    // println!("Received {:?} bytes of data", bytes_read); //response: Received Ok(690) bytes of data. But why did it print Ok(690) instead of 1s and 0s?
    let mut rdr = std::io::BufReader::new(&mut stream);
    
    loop{
        let mut lines = String::new();
        rdr.read_line(&mut lines).unwrap();
        if lines.trim().is_empty() {break;}
        println!("Request from client:\n{}", lines);
    }//where in this code did I convert 0s n 1s into UTF-8? 
    respond(stream); //send in lines later to add more functionality
    //but scope of lines ends in loop.. 
}
fn respond(mut stream: TcpStream){
    stream.write_all(b"HTTP/1.1 200 OK\r\n\r\nNamaste!\nFinally! You made it!!").unwrap();// ig b"" is converting out string into bytes"
    //can add if statements to return different webpages
}

fn main() {
    println!("Hello, world!");
    let listener = TcpListener::bind("127.0.0.1:7848").unwrap();
    loop {
        let stream = listener.accept();
    // for stream in listener.incoming(){
        match stream {
            Ok((mut stream, addr)) => {//because stream: {TcpStream, SocketAddr}
                println!("incoming connection from: {}", addr.to_string());
                handle_connection(stream);
            }
            Err(e) => { println!("connection failed") }
        }
    }
}

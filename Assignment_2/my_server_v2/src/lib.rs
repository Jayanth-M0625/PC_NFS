// use std::net::*;
// use std::io::{BufRead, Read, Write};
// use std::fs;
// fn handle_connection(mut stream: TcpStream) {
//     println!("Connection established!");
//     let mut rdr = std::io::BufReader::new(&mut stream);
//     let mut request = String::new(); 
//     loop{
//         let mut lines = String::new();
//         rdr.read_line(&mut lines).unwrap();
//         if lines.trim().is_empty() {break;}
//         request.push_str(&lines);
//     }
//     let response = response(&request); 
//     stream.write_all(response.as_bytes()).unwrap();
// }
// fn response(request: &String) -> String{
//     println!("Request from client:\n{}", request);
//     let get = "GET / HTTP/1.1\r\n";
//     let (status, file_path) = if request.starts_with(get) {
//         ("HTTP/1.1 200 OK", "index.html")
//     }
//     else {
//         ("HTTP/1.1 404 NOT FOUND", "404.html")
//     }
//     let content = fs::read_to_string(file_path).unwrap();
//     let response = format!(
//         "{}\r\nContent-Length: {}\r\n\r\n{}",
//         status,
//         content.len(),
//         content
//         );
//     response
// }
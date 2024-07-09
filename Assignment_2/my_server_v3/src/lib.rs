

pub mod server_deps{
    use std::net::*;
    use std::io::{BufRead, Read, Write};
    use std::fs;

    pub fn handle_connection(mut stream: TcpStream) {
        println!("Connection established!");
        let mut rdr = std::io::BufReader::new(&mut stream);
        let mut request = String::new(); 
        loop{
            let mut lines = String::new();
            rdr.read_line(&mut lines).unwrap();
            if lines.trim().is_empty() {break;}
            request.push_str(&lines);
        }
        let response = response(&request); 
        stream.write_all(response.as_bytes()).unwrap();
    }
    
    pub fn response(request: &String) -> String{
        println!("Request from client:\n{}", request);
        let get_1 = "GET / HTTP/1.1\r\n";
        let get_2 = "GET /game HTTP/1.1\r\n";
        let get_3 = "GET /new-game.html HTTP/1.1\r\n";
        let post_1 = "POST /redirect HTTP/1.1\r\n";
        let (status, file_path) = if request.starts_with(get_1) {
            ("HTTP/1.1 200 OK", "web_files/index.html")
        }
        else if request.starts_with(get_2) {
            ("HTTP/1.1 301 Moved Permanently\r\nLocation : /new-game.html\r\nConnection: close\r\n\r\n", "web_files/new-game.html")
        } 
        else if request.starts_with(get_3) {
            ("HTTP/1.1 200 OK", "web_files/new-game.html")
        }
        else if request.starts_with(post_1) {
            ("HTTP/1.1 200 OK", "web_files/503.html")
        }
        else {
            ("HTTP/1.1 404 NOT FOUND", "web_files/404.html")
        };
        let content = fs::read_to_string(file_path).unwrap();
        let response = format!(
            "{}\r\nContent-Length: {}\r\n\r\n{}",
            status,
            content.len(),
            content
            );
        response
    }
}
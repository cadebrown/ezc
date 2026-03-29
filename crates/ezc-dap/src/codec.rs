/// Content-Length framing for the Debug Adapter Protocol.
///
/// Each DAP message is preceded by HTTP-style headers terminated by `\r\n\r\n`:
///
/// ```text
/// Content-Length: 119\r\n
/// \r\n
/// {"seq":1,"type":"request","command":"initialize",...}
/// ```
///
/// This module provides `read_message` and `write_message` for that framing.
use std::io::{self, BufRead, Write};

use serde_json::Value;

/// Read a single DAP message from `reader`.
///
/// Parses the `Content-Length` header, reads exactly that many bytes of body,
/// and deserializes the body as JSON.
pub fn read_message<R: BufRead>(reader: &mut R) -> io::Result<Value> {
    // Read headers until blank line
    let mut content_length: Option<usize> = None;
    loop {
        let mut header_line = String::new();
        let n = reader.read_line(&mut header_line)?;
        if n == 0 {
            // Client closed stdin (e.g. VS Code disconnect) before sending a message.
            return Err(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "EOF while reading DAP headers",
            ));
        }
        let trimmed = header_line.trim_end_matches(['\r', '\n']);
        if trimmed.is_empty() {
            break; // blank line marks end of headers
        }
        if let Some(rest) = trimmed.strip_prefix("Content-Length: ") {
            content_length = rest.trim().parse().ok();
        }
        // Ignore other headers (e.g., Content-Type)
    }

    let len = content_length.ok_or_else(|| {
        io::Error::new(io::ErrorKind::InvalidData, "missing Content-Length header")
    })?;

    let mut body = vec![0u8; len];
    reader.read_exact(&mut body)?;

    serde_json::from_slice(&body)
        .map_err(|e| io::Error::new(io::ErrorKind::InvalidData, e.to_string()))
}

/// Write a single DAP message to `writer`.
///
/// Serializes `value` as JSON, wraps it in a `Content-Length` header, and
/// flushes the writer.
pub fn write_message<W: Write>(writer: &mut W, value: &Value) -> io::Result<()> {
    let body = serde_json::to_vec(value)
        .map_err(|e| io::Error::new(io::ErrorKind::InvalidData, e.to_string()))?;
    write!(writer, "Content-Length: {}\r\n\r\n", body.len())?;
    writer.write_all(&body)?;
    writer.flush()
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::json;

    #[test]
    fn roundtrip() {
        let msg = json!({ "seq": 1, "type": "request", "command": "initialize" });
        let mut buf = Vec::new();
        write_message(&mut buf, &msg).unwrap();

        let mut cursor = std::io::BufReader::new(buf.as_slice());
        let parsed = read_message(&mut cursor).unwrap();
        assert_eq!(parsed, msg);
    }

    #[test]
    fn eof_before_headers_is_unexpected_eof() {
        let mut cursor = std::io::BufReader::new(std::io::empty());
        let err = read_message(&mut cursor).expect_err("expected EOF");
        assert_eq!(err.kind(), std::io::ErrorKind::UnexpectedEof);
    }

    #[test]
    fn multiple_headers() {
        // Simulate a message with an extra header (should be ignored)
        let body = br#"{"seq":1}"#;
        let frame = format!(
            "Content-Type: application/vscode-jsonrpc; charset=utf-8\r\nContent-Length: {}\r\n\r\n",
            body.len()
        );
        let mut full = frame.into_bytes();
        full.extend_from_slice(body);
        let mut cursor = std::io::BufReader::new(full.as_slice());
        let parsed = read_message(&mut cursor).unwrap();
        assert_eq!(parsed["seq"], 1);
    }
}

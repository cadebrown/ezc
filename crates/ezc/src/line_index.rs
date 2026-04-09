/// Maps byte offsets to (line, column) positions in source text.
///
/// Lines and columns are 0-based internally. DAP and LSP both use 1-based values,
/// so callers should add 1 to the returned values when building protocol messages.
#[derive(Clone)]
pub struct LineIndex {
    /// Byte offset of the first character on each line.
    /// `line_starts[0]` is always 0. `line_starts[i]` is the offset right after
    /// the `\n` that ends line `i-1`.
    line_starts: Vec<usize>,
}

impl LineIndex {
    pub fn new(src: &str) -> Self {
        let mut starts = vec![0usize];
        for (i, b) in src.bytes().enumerate() {
            if b == b'\n' {
                starts.push(i + 1);
            }
        }
        LineIndex {
            line_starts: starts,
        }
    }

    /// Convert a byte offset to `(line, col)`, both 0-based.
    pub fn line_col(&self, offset: usize) -> (usize, usize) {
        let line = self
            .line_starts
            .partition_point(|&s| s <= offset)
            .saturating_sub(1);
        let col = offset.saturating_sub(self.line_starts[line]);
        (line, col)
    }

    /// Return the 0-based line number for a byte offset.
    pub fn line_of(&self, offset: usize) -> usize {
        self.line_starts
            .partition_point(|&s| s <= offset)
            .saturating_sub(1)
    }

    /// Return the byte offset of the start of a 0-based line.
    pub fn line_start(&self, line: usize) -> usize {
        self.line_starts.get(line).copied().unwrap_or(0)
    }

    /// Total number of lines in the source.
    pub fn line_count(&self) -> usize {
        self.line_starts.len()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn single_line() {
        let idx = LineIndex::new("hello world");
        assert_eq!(idx.line_col(0), (0, 0));
        assert_eq!(idx.line_col(5), (0, 5));
        assert_eq!(idx.line_col(10), (0, 10));
    }

    #[test]
    fn multi_line() {
        let src = "abc\ndef\nghi";
        let idx = LineIndex::new(src);
        assert_eq!(idx.line_col(0), (0, 0)); // 'a'
        assert_eq!(idx.line_col(3), (0, 3)); // '\n'
        assert_eq!(idx.line_col(4), (1, 0)); // 'd'
        assert_eq!(idx.line_col(7), (1, 3)); // '\n'
        assert_eq!(idx.line_col(8), (2, 0)); // 'g'
    }

    #[test]
    fn line_of() {
        let src = "a\nb\nc";
        let idx = LineIndex::new(src);
        assert_eq!(idx.line_of(0), 0);
        assert_eq!(idx.line_of(2), 1);
        assert_eq!(idx.line_of(4), 2);
    }
}

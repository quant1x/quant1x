use std::fmt;

#[derive(Debug)]
pub enum DeserializeError {
    Truncated { needed: usize, available: usize },
    InvalidUtf8,
    Other(String),
}

impl fmt::Display for DeserializeError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            DeserializeError::Truncated { needed, available } => {
                write!(f, "truncated: need {} available {}", needed, available)
            }
            DeserializeError::InvalidUtf8 => write!(f, "invalid utf8"),
            DeserializeError::Other(s) => write!(f, "{}", s),
        }
    }
}

impl std::error::Error for DeserializeError {}

impl From<String> for DeserializeError {
    fn from(s: String) -> Self {
        DeserializeError::Other(s)
    }
}

impl From<DeserializeError> for String {
    fn from(e: DeserializeError) -> Self {
        e.to_string()
    }
}

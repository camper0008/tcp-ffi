# tcp-ffi
Passing structs from C to Rust through a TCP connection.
Due to the way the variant tags are handled, you could probably also deserialize it into an enum if you gave it the proper values, but I decided not to.

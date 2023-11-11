# tcp-ffi
Passing structs between C and Rust through a TCP connection.

Due to the way the variant tags are handled, you could probably also de/serialize it into a rust enum if you gave it the proper values, but I decided not to.

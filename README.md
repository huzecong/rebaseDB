# rebaseDB

> A rough DBMS based on the RedBase project from CS346

A simple database management system supporting:

- Basic SQL commands
- B+ tree-based indexing
- Not-so-slow-but-not-too-fast-either query optimizer

See our (Chinese) [documentation](https://github.com/huzecong/rebaseDB/blob/master/doc/doc.md) for more details.

**On the RedBase framework:** For details of the Redbase framework, please refer to [CS346 RedBase Project Overview](https://web.stanford.edu/class/cs346/2015/redbase.html). For those wishing to implement their own DBMS using this framework, check out the `initial` branch.

## Usage

- Install dependencies: `glog` and `gflags`
- Clone this repo
- `mkdir cmake-build && cd cmake-build` (gather all the messy build files in one place)
- `cmake .. && make dbcreate && make redbase` (build the binaries)
- `./redbase`

For supported SQL commands, also refer to our (Chinese) documentation.

## References

See also:

- https://github.com/junkumar/redbase
- https://github.com/yifeih/redbase



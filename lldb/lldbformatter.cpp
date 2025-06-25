// https://lldb.llvm.org/resources/formatterbytecode.html#design-of-the-virtual-machine
#include <stack>

// All objects on the data stack must have one of the following data types. These data types are
// "host" data types, in LLDB parlance.
struct DataType {
  // Object and Type are opaque, they can only be used as a parameters of `call`.
  enum class tag {
    String,   // UTF-8
    Int,      // 64 bit
    UInt,     // 64 bit
    Object,   // Basically an SBValue
    Type,     // Basically an SBType
    Selector, // One of the predefine functions
  } tag;
};

enum class Instruction : uint8_t {
  // Stack operations

  // `(x -> x x)`
  dup = 0x00,
  // `(x y -> x)`
  drop = 0x01,
  // `(x ... UInt -> x ... x)`
  pick = 0x02,
  // `(x y -> x y x)`
  over = 0x03,
  // `(x y -> y x)`
  swap = 0x04,
  // `(x y z -> z x y)`
  rot = 0x05,

  // Control flow

  // `{`: push a code block address onto the control stack.
  // `}`: (technically not an opcode) syntax for end of code block.
  begin = 0x10,
  // `if (UInt -> )`: pop a block from the control stack, if the top of the data stack is nonzero,
  // execute it.
  If = 0x11,
  // `ifelse (UInt -> )`: pop two blocks from the control stack, if the top of the data stack is
  // nonzero, execute the first, otherwise the second.
  IfElse = 0x12,
  // `return`: pop the entire control stack and return.
  Return = 0x13,

  // Literals for basic types

  // `123u ( -> UInt)`: push an unsigned 64-bit host integer.
  UInt = 0x20,
  // `123 ( -> Int)`: push a signed 64-bit host integer.
  Int = 0x21,
  // `"abc" ( -> String)`: push a UTF-8 host string.
  String = 0x22,
  // `@strlen ( -> Selector)`: push one of the predefined function selectors. See call.
  Selector = 0x23,

  // Conversion operations

  // Arithmetic, logic, and comparison operations

  ADD = 0x30,
  SUB,
  MUL,
  DIV,
  SHL,
  SHR,
  NOT,
  OR,
  XOR,
  EQ,
  EQ2,
  LT,
  GT,
  LE,
  GE,

  // Function calls

  // `call (Object argN ... arg0 Selector -> retval)`
  Call = 0x60,
};

int main() { std::stack<uint8_t> data, control; }

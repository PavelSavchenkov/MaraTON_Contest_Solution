/*
 * 02 - Base64, serialization, lz4
 *
 * This example covers:
 * 1. Data types for binary data: td::BufferSlice, td::Slice, td::MutableSlice
 * 2. Reading and writing data as base64
 * 3. Cell serialization and deserialization
 * 4. lz4-compression
 * */

#include <iostream>
#include "td/utils/buffer.h"      // td::BufferSlice, td::Slice, td::MutableSlice
#include "td/utils/misc.h"        // td::buffer_to_hex
#include "common/util.h"          // td::base64_decode, td::str_base64_encode
#include "vm/boc.h"               // vm::std_boc_serialize, ...
#include "vm/cellslice.h"         // vm::load_cell_slice
#include "td/utils/lz4.h"         // td::lz4_compress, td::lz4_decompress

void example_slices() {
  // These types are used by many TON functions such as vm::std_boc_serialize, td::lz4_compress

  // td::Slice is a pointer to an array of bytes:
  unsigned char arr[8] = {11, 22, 33, 44, 55, 66, 77, 88};
  td::Slice slice{arr, 8};
  std::cout << "slice.size() = " << slice.size() << "\n";
  std::cout << "slice[3] = " << (int)slice[3] << "\n";
  CHECK(slice.data() == (char*)arr);

  // td::MutableSlice is the same thing, but the pointer is non-const
  td::MutableSlice mslice{arr, 8};
  mslice[3] = 100;
  std::cout << "arr[3] = " << (int)arr[3] << "\n\n";
  slice = mslice;  // td::MutableSlice can be converted to td::Slice

  // td::BufferSlice holds a buffer of data (like std::string), not just a pointer
  td::BufferSlice buf{slice};                 // This is a copy of arr
  td::Slice slice2 = buf;                     // Can be converted to td::Slice
  td::MutableSlice mslice2 = buf.as_slice();  // Can be converted to td::Slice

  // td::BufferSlice cannot be implicitly copied, only moved
  // td::BufferSlice buf2 = buf;          // WRONG
  td::BufferSlice buf2 = buf.clone();     // OK
  td::BufferSlice buf3 = std::move(buf);  // OK
}

void example_base64() {
  // Input and output data in the problem are in base64 format

  // Encode binary data -> base64
  unsigned char data[10] = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
  std::cout << "Data    = " << td::buffer_to_hex(td::Slice(data, 10)) << "\n";
  std::string encoded = td::str_base64_encode(td::Slice(data, 10));
  std::cout << "Encoded = " << encoded << "\n";

  // Decode base64 -> binary data
  td::BufferSlice data2 = td::base64_decode(encoded);
  std::cout << "Decoded = " << td::buffer_to_hex(data2) << "\n\n";
}

void example_serialize() {
  // This is a serialized cell
  td::BufferSlice data = td::base64_decode("te6ccuEBAgEAFAAOKAEIq80SNAEAFgAAAGQKFB4oMgAAq0rz6w==");
  std::cout << "Data = " << td::str_base64_encode(data) << "\n";

  // Deserialize
  td::Ref<vm::Cell> cell = vm::std_boc_deserialize(data).move_as_ok();
  // vm::std_boc_deserialize returns td::Result<td::Ref<vm::Cell>>, which can be td::Ref<vm::Cell> or Error
  // result.move_as_ok() returns the value or aborts the program (in case of Error)
  vm::load_cell_slice(cell).print_rec(std::cout);

  // Serialization
  td::BufferSlice serialized_31 = vm::std_boc_serialize(cell, 31).move_as_ok();
  td::BufferSlice serialized_0 = vm::std_boc_serialize(cell, 0).move_as_ok();
  std::cout << "Serialized (mode=31) = " << td::str_base64_encode(serialized_31) << "\n";
  std::cout << "Serialized (mode=0)  = " << td::str_base64_encode(serialized_0) << "\n\n";
  // Cell can be serialized with different modes
  // mode=0 is the most simple serialization
  // mode=31 is the most complex serialization with some extra data: index of cells, hashes of some inner cells etc
  // TON blocks are serialized using mode=31. That's because mode=31 makes it easier to parse data.
  // However, for block compression it makes sense to use a more compact mode.
}

void example_serialize_multi() {
  // Multiple roots can be serialized to one buffer
  td::BufferSlice data = td::base64_decode("te6ccuEBDAMANwIBAAoaJjA6RE5WXGJobgEEANgDBAMAQAgJCgsACKvNEjQBBAB9BAEEAEAFAQQAGwYBBAAIBwAEAAEAAgEAAgIAAgMAAgQ1ECq1");
  std::cout << "Data = " << td::str_base64_encode(data) << "\n";

  std::vector<td::Ref<vm::Cell>> roots = vm::std_boc_deserialize_multi(data).move_as_ok();  // 3 roots
  for (size_t i = 0; i < roots.size(); ++i) {
    std::cout << "roots[" << i << "] = ";
    vm::load_cell_slice(roots[i]).print_rec(std::cout);
  }

  td::BufferSlice serialized = vm::std_boc_serialize_multi(roots, 31).move_as_ok();
  std::cout << "Serialized = " << td::str_base64_encode(serialized) << "\n\n";
}

void example_lz4() {
  // This uses lz4 library (https://github.com/lz4/lz4)
  td::BufferSlice data = td::base64_decode("LS0tLS0tLS0tLS0tLS0tLS0tLS0tLSBxaHp2IGVycXh2IGhvZ2h1IGZ1bHZzIGdodmZ1bGVoIHhxZnJ5aHUgcGhnZG8gdnpydWcgcGh1ZmIga3J4dSB6a2RvaCBpaHdmayBzcmh3IGZkd2hqcnViIG14cWxydSBreHBydSB1bHlodSBzbGpocnEgc3VsZmggcGR3d2h1IGd4d2Igb2xkdSB3dWRqbGYgd3VkaWlsZiAtLS0tLS0tLS0tLS0tLS0tLS0tLS0t");
  std::cout << "Data         = " << td::str_base64_encode(data) << "\n";

  // Compress data using lz4
  td::BufferSlice compressed = td::lz4_compress(data);
  std::cout << "Compressed   = " << td::str_base64_encode(compressed) << "\n";

  // Decompress data using lz4
  // (2 << 20) is the maximum size of decompressed data
  // When working with TON blocks, it can be set to (2 << 20) because the upper limit for block size is 2MB
  // (and actual limits and block sizes are much lower)
  td::BufferSlice decompressed = td::lz4_decompress(compressed, 2 << 20).move_as_ok();
  std::cout << "Decompressed = " << td::str_base64_encode(decompressed) << "\n";
}

int main() {
  std::cout << "------- Slices -------\n";
  example_slices();
  std::cout << "------- Base64  ------\n";
  example_base64();
  std::cout << "------- Serialize ------\n";
  example_serialize();
  std::cout << "------- Serialize multi ------\n";
  example_serialize_multi();
  std::cout << "------- LZ4 ------\n";
  example_lz4();
  return 0;
}

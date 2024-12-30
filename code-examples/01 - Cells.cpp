/*
 * 01 - Cells
 *
 * This file provides examples for working with cells.
 * A cell is an object that stores up to 1023 bits of data and up to 4 references to other cells.
 * Cell builder is an object used to build new cells.
 * Cell slice is an object used to read data from cells.
 *
 * You can be familiar with cells, cell slices and cell builders if you participated in FunC contests.
 */

#include <iostream>
#include "vm/cells/CellBuilder.h"
#include "vm/cells/CellSlice.h"
#include "vm/cellslice.h"

int main() {
  // Creating a new cell
  vm::CellBuilder cb;
  cb.store_long(100, 32);  // Store a 32-bit integer
  unsigned char bytes[5] = {10, 20, 30, 40, 50};
  cb.store_bytes(td::Slice(bytes, 5));  // Store 5 bytes
  cb.store_zeroes(16);  // Store 16 0-bits
  td::Ref<vm::Cell> cell = cb.finalize();  // Create a cell from cell builder

  // Creating another cell with a reference to the first cell
  cb.reset();
  cb.store_long(0xabcd1234, 32);
  cb.store_ref(cell);  // Store a reference
  std::cout << "Cell builder size: " << cb.size() << " " << cb.size_refs() << "\n\n"; // This cb holds 32 bits and 1 ref
  td::Ref<vm::Cell> cell2 = cb.finalize();  // Create a new cell
  // You can see more CellBuilder methods in include/crypto/vm/cells/CellBuilder.h

  // Let's check cell hash and depth
  std::cout << "Cell hash = " << cell2->get_hash().to_hex() << "\n";
  std::cout << "Cell depth = " << cell2->get_depth() << "\n\n";

  // Reading a cell
  vm::CellSlice cs = vm::load_cell_slice(cell2);

  // Print cell slice recursively:
  // x{ABCD1234}
  //  x{00000064AABBCCDD000000}
  cs.print_rec(std::cout);
  std::cout << "\n";

  // Reading bits from cell slice
  std::cout << "Size = " << cs.size() << " " << cs.size_refs() << "\n";  // 32 bits, 1 ref
  std::cout << "Data = " << cs.fetch_long(16) << "\n";  // Read 16 bits
  std::cout << "Size = " << cs.size() << " " << cs.size_refs() << "\n";  // 16 bits, 1 ref remaining
  std::cout << "Data = " << cs.prefetch_long(16) << "\n";  // Read 16 bits withoud advancing the cell slice
  std::cout << "Size = " << cs.size() << " " << cs.size_refs() << "\n\n";  // Still 16 bits, 1 ref remaining

  // Reading the reference
  vm::CellSlice cs2 = vm::load_cell_slice(cs.fetch_ref());
  std::cout << "Size = " << cs2.size() << " " << cs2.size_refs() << "\n";  // 32 bits, 1 ref
  std::cout << "Data = " << cs2.fetch_long(32) << "\n";  // Read 32-bit integer
  unsigned char bytes2[5];
  cs2.fetch_bytes(td::MutableSlice(bytes2, 5));  // Read 5 bytes
  for (unsigned char x : bytes2) {
    std::cout << "Data = " << (unsigned int)x << "\n";
  }
  std::cout << "\n";
  // You can see more CellSlice methods in include/crypto/vm/cells/CellSlice.h

  // td::Ref<> is similar to a std::smart_ptr<>
  // vm::CellSlice and other objects are often stored in td::Ref<>
  td::Ref<vm::CellSlice> cs3 = vm::load_cell_slice_ref(cell2);
  std::cout << "Size = " << cs3->size() << "\n";
  // td::Ref<> contents are immutable:
  std::cout << "Data = " << cs3->prefetch_long(32) << "\n";  // OK
  // std::cout << "Data = " << cs3->fetch_long(32) << "\n";  // Compilation error
  
  // You can modify contents td::Ref<> using .write()
  // Other copies td::Ref<> that point to the same object remain unchanged
  vm::CellSlice& cs4 = cs3.write();
  std::cout << "Data = " << cs4.fetch_long(32) << "\n\n";

  // Special (exotic) cells
  // Special cell is technically a normal cell with a flag special=true
  // Special cells have some restrictions on what they can store. Check docs for more information:
  // https://docs.ton.org/develop/data-formats/exotic-cells
  vm::CellBuilder cb_spec;
  cb_spec.store_long(2, 8);
  cb_spec.store_long(0x1234567812345678LL, 64);
  cb_spec.store_long(0x1234567812345678LL, 64);
  cb_spec.store_long(0x1234567812345678LL, 64);
  cb_spec.store_long(0x1234567812345678LL, 64);
  td::Ref<vm::Cell> cell_spec = cb_spec.finalize(true);  // Create a special cell

  // This throws an exception!
  // vm::CellSlice cs_spec = vm::load_cell_slice(cell_spec);
  
  bool spec;
  vm::CellSlice cs_spec = vm::load_cell_slice_special(cell_spec, spec);
  std::cout << "Is_special = " << spec << "\n";  // spec == true. For non-special cells, spec == false
  cs_spec.print_rec(std::cout);

  return 0;
}

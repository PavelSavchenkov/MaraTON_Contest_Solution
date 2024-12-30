/*
 * 06 - Dictionary
 *
 * Dictionary is a map from n-bit integers to cell slices. You can work with dictionaries using class vm::Dictionary.
 * Empty dictionary is represented as null cell (td::Ref<vm::Cell>.is_null())
 * Non-empty dictionary has a non-null root cell.
 * Internally, dict is a compressed trie.
 *
 * Dictionaries are usually stored as (Maybe ^Cell), meaning that it is either 0-bit (empty) or 1-bit and a reference
 * to the root (non-empty).
 *
 * TLB type (Hashmap n X) is a non-empty dictionary with key length of n bits any value type X.
 * (HashmapE n X) is (Maybe ^(Hashmap n X)).
 */

#include <iostream>
#include "common/bitstring.h"
#include "vm/cells/CellBuilder.h"
#include "vm/dict.h"

int main() {
  // Let's create a dictionary
  vm::Dictionary dict{32};  // Key length is 32
  CHECK(dict.get_root_cell().is_null());  // Initially empty
  // Add values. dict.set takes td::ConstBitPtr as key, td::Ref<vm::CellSlice> as value
  dict.set(td::BitArray<32>(1), vm::CellBuilder().store_long(0xaaaaaaaa, 32).as_cellslice_ref());
  dict.set(td::BitArray<32>(2), vm::CellBuilder().store_long(0x12345678, 32).as_cellslice_ref());
  dict.set(td::BitArray<32>(10), vm::CellBuilder().store_long(0x87654321, 32).as_cellslice_ref());

  // Get values
  td::Ref<vm::CellSlice> cs = dict.lookup(td::BitArray<32>(1));
  std::cout << "dict[1] = ";
  cs->print_rec(std::cout);
  cs = dict.lookup(td::BitArray<32>(5));  // non-existing value
  CHECK(cs.is_null());
  std::cout << "dict[5] = null\n\n";

  // Dictionary can also store refs instead of cell slices
  td::Ref<vm::Cell> cell = vm::CellBuilder().store_long(0xaaaabbbbccccddddLL, 64).finalize();
  dict.set_ref(td::BitArray<32>(15), cell);
  td::Ref<vm::Cell> cell2 = dict.lookup_ref(td::BitArray<32>(15));
  CHECK(cell->get_hash() == cell2->get_hash());
  // set_ref(cell) is equivalent to set(cs) where cs is a slice with a single reference to the cell
  // In block tlb, types such as (Hashmap 32 StoragePrices) mean that we store values as cell slices,
  // types such as (HashmapE 96 ^TopBlockDescr) mean that we store values as cells

  // Remove value
  dict.lookup_delete(td::BitArray<32>(2));

  // Iterate over all values
  bool ok = dict.check_for_each([&](td::Ref<vm::CellSlice> value, td::ConstBitPtr key, int key_len) -> bool {
    CHECK(key_len == 32);
    int key_int = key.get_int(32);
    std::cout << "dict[" << key_int << "] = ";
    value->print_rec(std::cout);
    return true;  // If we return false, check_for_each will stop end return false
  });
  CHECK(ok);
}

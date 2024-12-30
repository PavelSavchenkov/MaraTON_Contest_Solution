/*
 * 04 - TLB 2
 */

#include <iostream>
#include "block/block-auto.h"
#include "vm/cells/CellBuilder.h"
#include "vm/cells/CellSlice.h"

int main() {
	// Let's build a MsgAddressInt (addr_std):
	// addr_std$10 anycast:(Maybe Anycast) workchain_id:int8 address:bits256 = MsgAddressInt;
	// addr_var$11 anycast:(Maybe Anycast) addr_len:(## 9) workchain_id:int32 address:(bits addr_len) = MsgAddressInt;
	block::gen::MsgAddressInt::Record_addr_std addr_rec;
	addr_rec.workchain_id = 0;
	// address is td::BitArray<256>
	// See include/crypto/common/bitstring.h
	CHECK(addr_rec.address.from_hex("abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd"));
	// Put 0-bit in anycast:(Maybe Anycast)
	addr_rec.anycast = vm::CellBuilder().store_zeroes(1).as_cellslice_ref();

	// This serializes addr_rec and appends it to CellBuilder
	vm::CellBuilder cb;
	bool success = block::gen::MsgAddressInt().pack(cb, addr_rec);  // Returns true on success
	CHECK(success);
	vm::CellSlice cs = cb.as_cellslice();  // Same as cb.finalize() -> vm::load_cell_slice
	// Print the result
	cs.print_rec(std::cout);
	block::gen::MsgAddressInt().print(std::cout, cs);
	// cell_pack packs to cell
	td::Ref<vm::Cell> cell;
	success = block::gen::MsgAddressInt().cell_pack(cell, addr_rec);
	CHECK(success);
	block::gen::MsgAddressInt().print_ref(std::cout, cell);
	std::cout << "\n";

	// How to parse MsgAddressInt, considering that it has two possible variants (addr_std, addr_var):
	block::gen::MsgAddressInt::Record_addr_std rec1;
	block::gen::MsgAddressInt::Record_addr_var rec2;
	bool ok1 = block::gen::MsgAddressInt().cell_unpack(cell, rec1);  // Returns true
	bool ok2 = block::gen::MsgAddressInt().cell_unpack(cell, rec2);  // Returns false
	CHECK(ok1 && !ok2);
	std::cout << "Workchain = " << rec1.workchain_id << "\n";
	std::cout << "Address = " << rec1.address.to_hex() << "\n";
}

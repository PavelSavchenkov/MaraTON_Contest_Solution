/*
 * 05 - Int256
 *
 * td::RefInt256 is a 257-bit (!) integer type that holds an integer in [-2^256, 2^256).
 * It corresponds to "int" in FunC.
 */

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "common/refint.h"
#include "vm/cells/CellBuilder.h"
#include "vm/cells/CellSlice.h"
#include "block/block-parse.h"  // block::tlb::Grams

int main() {
	td::RefInt256 x = td::zero_refint();  // 0
	td::RefInt256 y = td::make_refint(1234);  // From int
	td::RefInt256 z = td::make_refint(100);  // From int
	td::RefInt256 t = td::dec_string_to_int256(std::string("123456789123456789987654321"));  // From string
	std::cout << x->to_dec_string() << " " << y->to_dec_string() << " " << z->to_dec_string() << " " << t->to_dec_string() << "\n\n";

	// Arithmetic operations are available
	std::cout << y + z << "\n";
	std::cout << y - z << "\n";
	std::cout << y / z << "\n";
	std::cout << y % z << "\n";
	std::cout << t * z * z << "\n\n";

	// Invalid operations (overflows, divide by zero) return NaN
	std::cout << "Division by zero: " << y / x << "\n";
	std::cout << "Is non-NaN: " << (y / x)->is_valid() << "\n\n";

	// Convert to int
	long long y_int = y->to_long();
	std::cout << y_int << "\n\n";

	// Store to cell (fixt bit length)
	vm::CellBuilder cb;
	cb.store_int256(*y, 32, /* signed = */ false);
	cb.as_cellslice().print_rec(std::cout);
	std::cout << "\n";

	// Store to cell as (VarUInteger 16), aka Coins or Grams
	// This is a default format for storing toncoin balance.
	// This corresponds to store_coins in FunC.
	cb.reset();
	bool success = block::tlb::Grams().store_integer_value(cb, *t);
	CHECK(success);
	vm::CellSlice cs = cb.as_cellslice();
	cs.print_rec(std::cout);

	// Read Coins
	td::RefInt256 t2 = block::tlb::Grams().as_integer_skip(cs);
	std::cout << "Coins = " << t2->to_dec_string() << "\n";
}

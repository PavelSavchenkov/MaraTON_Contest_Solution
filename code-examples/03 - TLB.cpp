/*
 * 03 - TLB
 *
 * Data in TON (for example, blocks) are stored in cells.
 * Data format is described using TLB schemes. See block.tlb for schemes of TON blocks and other data types.
 * block.tlb format is described in the documentation: https://docs.ton.org/develop/data-formats/tl-b-language
 *
 * Let's consider some examples:
 *
 * foo#a0 a:uint8 b:# c:Bool d:(## 15) e:^Cell = Foo;
 * Foo consists of:
 *   Byte 0xA0
 *   8-bit uint a
 *   32-bit uint b (# is the same as uint32)
 *   1 bit c
 *   15-bit uint d (## 15 is 15-bit uint)
 *   Reference to cell e
 *
 * var_uint$_ {n:#} len:(#< n) value:(uint (len * 8)) = VarUInteger n;
 * VarUInteger n is a generic type that has integer parameter n:
 *   $_ - no prefix (such as 0xA0 in the prevoius example)
 *   {n:#} - not a field. This just signifies that n is uint32
 *   len:(#< n) - unsigned integer in [0..n). Bit length is the minimum possible x such that 2^x >= n
 *   value:(uint (len * 8)) - unsigned integer of length len * 8 bits
 * VarUInteger 16 (also called Coins or Grams) is usually used to store toncoin amounts (in nanotons)
 *
 * addr_none$00 = MsgAddressExt;
 * addr_extern$01 len:(## 9) external_address:(bits len) = MsgAddressExt;
 * MsgAddressExt is a type that can be either:
 *   addr_none - bits 00
 *   addr_extern:
 *     Bits 01
 *     9-bit uint len
 *     address (len bits)
 *
 * int_msg_info$0 ihr_disabled:Bool bounce:Bool bounced:Bool src:MsgAddressInt dest:MsgAddressInt
 *   value:CurrencyCollection ihr_fee:Grams fwd_fee:Grams
 *   created_lt:uint64 created_at:uint32 = CommonMsgInfo;
 * This example shows how one types can be used in decribing other types.
 *
 * ... = Account;
 * account_descr$_ account:^Account last_trans_hash:bits256 last_trans_lt:uint64 = ShardAccount;
 * Here account:^Account is a reference to a cell with Account
 *
 * bar#b0 x:(Maybe ^Cell) = Bar;
 * Here x:(Maybe ^Cell) is either:
 *   Bit 0
 *   Bit 1 and then a reference to a cell
 *
 * baz#c1 x:uint32 y:^[ a:uint32 b:uint64 c:^Cell ] = Baz;
 * Here y is a reference to a cell with scheme described inside []
 *
 * aaa#b0 flags:# x:flags . 0?uint32 y:flags . 1?^Cell = Aaa;
 * Here fields x and y exist only if the corresponding bits in flags (uint32) are set
 *
 * ===========================================================================================================
 *
 * Data in cells can be parsed and stored automatically using parsers in block/block-auto.h
 */

#include <iostream>
#include <sstream>
#include "block/block-auto.h"
#include "common/util.h"
#include "vm/boc.h"
#include "vm/cells/CellSlice.h"

std::string block_base64 = "te6ccuECGAEABCIAABwAxADeAXACBAKgAzwDYgN0A9oEQAR4BVQFoAXYBfEGzAcZBzoHkgfqCDYIPghFBBAR71Wq/////QECAwQCoJvHqYcAAAAAhAEBfdQUAAAAAAIAAAAAQAAAAAAAAABm8ndHAAAX2n/GH4AAABfaf8YfgZTPzi4ABNAsAWQyHgFkMcLEAAAACQAAAAAAAAHuBQYCEbjkjftA7msoBAcICooEZVuLkqfR2Hvr0LtEtjGy8P4+8vY9IOx04zPqtjFzdcxlsrE36zMVNjNENOXVdRr+mXscIb8AEbALbnU5fWaMfQH9Af0JCgOJSjP2/dHwlDWZlNhirK1XNPFMuQ9WC/nuPVo80SIU2tqj1eaLvqJiJHVRqGnIONmEJs/M2hAXNo2nNdDwj89i1mqXEn1AFhcXAJgAABfaf5hYxAFkMiJoYoWAvrkEPQa/hoZY9RJI3LgfOaYiS6wB4WvyauhE8nZGcj+xsTW8Hfar5kYUHQM6mhvH0shCfV3RfPFiixroAJgAABfaf7bdQQF91BM/imGJaFpDXoGoZ1YnBaCFwxkHn4dU9ocvCUFvv0szA2ri49NxN+jCY35LSW1hgZHexOTm2AU83Pgf3Nlw2+02ACFxZJvp4hFsA4sk308Qi2AACAANABA7msoACCNbkCOv4v////0CAAAAAEAAAAAAAAAAAX3UEwAAAABm8ndFAAAX2n+23UEBZDIeIAsPDCNbkCOv4v////0CAAAAAEAAAAAAAAAAAX3UFAAAAABm8ndHAAAX2n/GH4EBZDIeIA4PECIvgAAL7T+mBsFgAAAAAAAAAACAAAAAAADAEQ0A1wAAAAAAAAAA//////////9xZJvp4hFsA4u3zf/97yiQAAF9p/mFjEAWQyImhihYC+uQQ9Br+Ghlj1EkjcuB85piJLrAHha/Jq6ETydkZyP7GxNbwd9qvmRhQdAzqaG8fSyEJ9XdF88WKLGuiChIAQFZJtG1i3TxKYGWanNrcGsXzidi1ej4Eq4RNl2N4QbAtgABIi+AAAvtP6YGwWAAAAAAAAAAAIAAAAAAAMAREiERgcWSb6eIRbAQFQDXAAAAAAAAAAD//////////3Fkm+niEWwDi7fOB3FIaJAAAX2n+YWMQBZDIiaGKFgL65BD0Gv4aGWPUSSNy4HzmmIkusAeFr8mroRPJ2RnI/sbE1vB32q+ZGFB0DOpobx9LIQn1d0XzxYosa6IKEgBAbRqWQYnyaInQ6J+EuL6mYBpfoNdp3ZkUdC24ICJMks0AAMCGa0wAAAAAAAAAACyGRATFABTr4AABfaf2qSAryTNYHEwWzeMeVooosgSIIRQFax7NWOLCCjpMoNsnJzgAFOogAAF9p/TA2CyWB6hVBm6xNVPXQA4ifKFRp6X7zyHCZVQB07DpCtyIyAoSAEBM1wMRuZ3PvnKd5ikowagWoNvgD50Q9EF/DQzaCG5gioB+wADACAAAQLAAi5n";

int main() {
	// This is a block. Let's parse it
	td::Ref<vm::Cell> block_root = vm::std_boc_deserialize(td::base64_decode(block_base64)).move_as_ok();

	// block::gen::Block() is an object that represents Block scheme from block.tlb
	// This validates that block_root matches Block scheme
	std::cout << "Validate = " << block::gen::Block().validate_ref(10'000'000, block_root) << "\n";
	// 10'000'000 is the limit for the number of cells to visit during validation
	// Default for validate_ref is 1024, which is not enough for a block
	// 10'000'000 is enough (the same value is used in the actual validator)

	vm::CellSlice cell_slice = vm::load_cell_slice(block_root);

	// Same thing, but for cell slice:
	int ops = 10'000'000;
	std::cout << "Validate cs = " << block::gen::Block().validate(&ops, vm::load_cell_slice(block_root)) << "\n";
	// This actually changes ops
	std::cout << "Remaining ops = " << ops << "\n";

	// This prints the block according to the TLB scheme
	// Cutting the output because it's pretty long
	std::ostringstream ss;
	//block::gen::Block().print_ref(ss, block_root);
	block::gen::Block().print(ss, vm::load_cell_slice(block_root)); // same thing, but for cell slice

	auto printed = ss.str();
	std::cout << "PRINTED.size() = " << printed.size() << "\n";
	std::cout << "Block = " << printed << "\n";

	// Let's parse the block. This is done using block::gen::Block::Record
	block::gen::Block::Record block_rec;
	bool success = block::gen::Block().cell_unpack(block_root, block_rec);  // Returns true on success
	CHECK(success);

	// Same thing, but for cell slice (note that cs advances after the operation):
	// vm::CellSlice cs = vm::load_cell_slice(block_root);
	// block::gen::Block().unpack(cs, block_rec);

	// Block TLB scheme is:
	// block#11ef55aa global_id:int32 info:^BlockInfo value_flow:^ValueFlow state_update:^(MERKLE_UPDATE ShardState) extra:^BlockExtra = Block;
	// Let's parse further to get BlockInfo
	block::gen::BlockInfo::Record info_rec;
	block::gen::BlockInfo().cell_unpack(block_rec.info, info_rec);
	std::cout << "Unixtime = " << info_rec.gen_utime << "\n";
	std::cout << "Start LT = " << info_rec.start_lt << "\n";
	std::cout << "Masterchain = " << !info_rec.not_master << "\n";

	// Let's parse shard:ShardIdent
	block::gen::ShardIdent::Record shard_rec;
	// info_rec.shard is td::Ref<vm::CellSlice>
	// Use .write() to convert it to vm::CellSlice&
	block::gen::ShardIdent().unpack(info_rec.shard.write(), shard_rec);
	std::cout << "Workchain = " << shard_rec.workchain_id << "\n";
}

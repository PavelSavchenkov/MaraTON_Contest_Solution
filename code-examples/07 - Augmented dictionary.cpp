/*
 * 07 - Augmented dictionary
 *
 * Augmented dictionary is a dictionary that stores "augmentation data" in every node.
 * This allows to efficiently aggregate some data over the whole dict.
 *
 * For example, accounts in TON are stored in a dictionary (address -> account).
 * Each node of the dictionary contains sum of balances of the accounts in its subtree.
 * This allows to efficiently calculate the total balance of accounts in a shard.
 *
 * TLB type of an augmented dictionary is (HashmapAugE n X Y).
 * n is key length, X is data type, Y is augmentation data type.
 * (HashmapAug n X Y) is a non-empty dict.
 * (HashmapAugE n X Y) is [ dict:(Maybe ^(HashmapAug n X Y)) extra:Y ].
 *
 * You can work with such dictionaries using vm::AugmentedDictionary.
 * You need a special AugmentationData object, which defines how to calculate augmentation data:
 *   eval_empty: returns aug data for an empty dict
 *   eval_leaf: returns aug data for a single value
 *   eval_fork: combines two aug data into one
 * AugmentationData objects for types in block.tlb is available in block/block-parse.h
 *
 * ==============================================================================================
 * Consider an example: dictionary of accounts
 *
 * currencies$_ grams:Grams other:ExtraCurrencyCollection = CurrencyCollection;
 * depth_balance$_ split_depth:(#<= 30) balance:CurrencyCollection = DepthBalanceInfo;
 * _ (HashmapAugE 256 ShardAccount DepthBalanceInfo) = ShardAccounts;
 *
 * Aug data stores total TON balance (grams:Grams) and some other information.
 */

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "block/block-auto.h"
#include "common/bitstring.h"
#include "vm/boc.h"
#include "vm/cells/CellBuilder.h"
#include "vm/cells/CellSlice.h"
#include "vm/dict.h"
#include "block/block-parse.h"
#include "common/util.h"

int main() {
	// This is a ShardAccounts augmented dictionary
	// 256 is key length
	// Aug_ShardAccounts implements computing aug data for this dictionary
	// cell stores (HashMapAugE 256 ShardAccount DepthBalanceInfo)
	td::Ref<vm::Cell> cell = vm::std_boc_deserialize(td::base64_decode("te6ccgECHgEABYcAAQuBEI6NcdABAgsAiEdGuOgCAwILAIdzip7IBAUCCwCA07waKBESAgtQIdzWU3IGBwIJAGAxUQgNDgIHcAC3IAgJAZy/CeysHWm5cUMLANoz9GFh9gsyCr+DeKBX4lNos52IrrgIdzWUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMAZW+FyhR6JQmK6O7rsB2XXcFDkWoCcKg2Sgn5Gee3CwLFIAsgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgKAZW+PZFI/GWcbc6POyKqJxP0XSWrwBE8NJRZyeDeZ3EobMAhQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgLAGHAAgXKFHolCYro7uuwHZddwUORagJwqDZKCfkZ57cLAsUgAAAAAAAAAAAAAAAAAFkEAGHAAh9kUj8ZZxtzo87IqonE/RdJavAETw0lFnJ4N5ncShswAAAAAAAAAAAAAAAAAEKEAGfAAyeysHWm5cUMLANoz9GFh9gsyCr+DeKBX4lNos52IrrgAAAAAAAAAAAAAAAAAQ7msoAEAZm/cgKDpumNAoo4pl3iN3o+svyYYtBC7Imldro52MKOZHICA+gAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQA8Bm79Qp2ltf6tNkgFyZ2AZQJEfIqMFs3vh7KNBkubOeBI9rAMBhqAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQBAAY8AFkBQdN0xoFFHFMu8Ru9H1l+TDFoIXZE0rtdHOxhRzI5AAAAAAAAAAAAAAAAAAgPoEAGXABoU7S2v9WmyQC5M7AMoEiPkVGC2b3w9lGgyXNnPAke1gAAAAAAAAAAAAAAAAAMBhqAQCCmwHMXsgExQCCwCAwKRoKBcYAZe+8rf7Die25bJtz9F1yiNELsW6d6oJGTVf0p68aCqr7MAInEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABFQGZvt+Pzy8MfqInaJzgXAZ5LlcjS8AYfogEOGlokZKv8wegDmJaAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEWAGPAC2Vv9hxPbctk25+i65RGiF2LdO9UEjJqv6U9eNBVV9mAAAAAAAAAAAAAAAAAAInEBABlwAu/H55eGP1ETtE5wLgM8lyuRpeAMP0QCHDS0SMlX+YPQAAAAAAAAAAAAAAAAADmJaAEAgkAYehIKBkaAZ2/fxGR2LrXVTWIFcqhCv/g/rzCPFvdCWw5bOH+mAoom+YEBfXhAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABAHQGavy8b3Vazf1rcYMNVq5Wve3V078lXj2x32vyEkZKpEVbIBh6EgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAbAZa/EQHJae5u+qE0VsO7+vx6gMUU45/wNSZjx75EUIqTUIwCAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAcAGXADLxvdVrN/Wtxgw1Wrla97dXTvyVePbHfa/ISRkqkRVsgAAAAAAAAAAAAAAAAAMPQkAQAYcANRAclp7m76oTRWw7v6/HqAxRTjn/A1JmPHvkRQipNQjAAAAAAAAAAAAAAAAAAQEQAZ8AP+IyOxda6qaxArlUIV/8H9eYR4t7oS2HLZw/0wFFE3zAAAAAAAAAAAAAAAAABAX14QAQ=")).move_as_ok();

	block::tlb::Aug_ShardAccounts aug_ShardAccounts;  // Note: AugmentedDictionary takes aug_ShardAccounts as const&
	vm::AugmentedDictionary accounts_dict{ vm::load_cell_slice_ref(cell), 256, aug_ShardAccounts };

	// This is aug data for root
	td::Ref<vm::CellSlice> root_aug_data = accounts_dict.get_root_extra();
	std::cout << "Root aug data = ";
	block::gen::DepthBalanceInfo().print(std::cout, root_aug_data);  // Note: total blance is 1111111111
	std::cout << "\n";

	// Delete a value:
	td::Bits256 key;
	CHECK(key.from_hex("CBC6F755ACDFD6B71830D56AE56BDEDD5D3BF255E3DB1DF6BF212464AA4455B2"));
	td::Ref<vm::CellSlice> cs = accounts_dict.lookup_delete(key);
	block::gen::ShardAccount().print(std::cout, cs);  // Note: blance is 1000000
	std::cout << "\n";

	// New aug data for root
	root_aug_data = accounts_dict.get_root_extra();
	std::cout << "Root aug data = ";
	block::gen::DepthBalanceInfo().print(std::cout, root_aug_data);  // Note: total blance is 1110111111
	std::cout << "\n";

	// Store accounts_dict as HashmapAugE:
	vm::CellBuilder cb;
	CHECK(accounts_dict.append_dict_to_bool(cb));
	CHECK(block::gen::HashmapAugE(256, block::gen::ShardAccount(), block::gen::DepthBalanceInfo()).validate_ref(cb.finalize()));

	// Store empty accounts_dict as HashmapAugE
	vm::AugmentedDictionary accounts_dict_empty{ 256, aug_ShardAccounts };
	cb.reset();
	CHECK(accounts_dict_empty.append_dict_to_bool(cb));
	std::cout << "Empty dict = ";
	CHECK(block::gen::HashmapAugE(256, block::gen::ShardAccount(), block::gen::DepthBalanceInfo()).print_ref(std::cout, cb.finalize()));
}

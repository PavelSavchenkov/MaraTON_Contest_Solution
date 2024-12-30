#include <memory>

#include "utils.h"

namespace Serializator {
template<typename T>
using Ptr = std::shared_ptr<T>;

struct MyCell {
    unsigned cnt_refs{-1u};
    std::array<unsigned, 4> refs{};
    std::array<td::Ref<vm::Cell>, 4> refs_as_cells{};

    bool is_special{};

    std::basic_string<uint8_t> data{};
    unsigned cnt_bits{-1u};

    CellType get_type() const {
        if (!is_special) {
            return CellType::Ordinary;
        }
        const auto type = static_cast<CellType>(data.at(0));
        CHECK(type != CellType::Ordinary);
        return type;
    }

    MyCell() {
    }

    explicit MyCell(td::Ref<vm::Cell> cell) {
        vm::Cell::LoadedCell cell_loaded = cell->load_cell().move_as_ok();
        CellType cell_type = cell_loaded.data_cell->special_type();
        is_special = (cell_type != CellType::Ordinary);
        cnt_refs = cell_loaded.data_cell->get_refs_cnt();

        cnt_bits = cell_loaded.data_cell->size();

        vm::CellSlice cell_slice = cell_to_slice(cell);
        unsigned bits_left = cnt_bits;
        while (bits_left >= 8) {
            uint8_t byte = cell_slice.fetch_ulong(8);
            data.push_back(byte);
            bits_left -= 8;
        }
        if (bits_left > 0) {
            CHECK(bits_left < 8);
            uint8_t last_byte = cell_slice.fetch_ulong(bits_left);
            last_byte <<= 8 - bits_left;
            data.push_back(last_byte);

            CHECK(((last_byte >> (7 - bits_left)) & 1) == 0);
            // last_byte ^= 1u << (7 - bits_left);
        } else {
            CHECK(bits_left == 0);
        }

        const unsigned cnt_refs = cell_loaded.data_cell->get_refs_cnt();
        CHECK(cnt_refs <= 4);
        for (unsigned i = 0; i < cnt_refs; i++) {
            td::Ref<vm::Cell> ref = cell_loaded.data_cell->get_ref(i);
            refs_as_cells[i] = ref;
        }

        if (get_type() == CellType::MerkleUpdate) {
            data.resize(1);
            cnt_bits = 8;
        }
    }

    uint8_t d1() const {
        return cnt_refs + 8 * is_special;
    }

    uint8_t d2() const {
        unsigned desc = cnt_bits / 8;
        desc *= 2;
        if (cnt_bits % 8 != 0) {
            desc ^= 1u;
        }
        return desc;
    }

    bool operator==(const MyCell &rhs) const {
        if (cnt_bits != rhs.cnt_bits) return false;
        if (data != rhs.data) return false;
        if (is_special != rhs.is_special) return false;
        if (cnt_refs != rhs.cnt_refs) return false;
        if (refs != rhs.refs) return false;
        return true;
    }

    void print() const {
        std::cout << "MyCell|cnt_bits=" << cnt_bits;
        std::cout << ", is_special=" << is_special << ", cnt_refs=" << cnt_refs;
        std::cout << ", data=";
        for (const auto &c: data) {
            std::cout << std::bitset<8>(c) << " ";
        }
        std::cout << std::dec;
        std::cout << ", refs=";
        for (const auto &r: refs) {
            std::cout << r << " ";
        }
        std::cout << "|" << std::endl;
    }
};

void push_as_bytes(std::basic_string<uint8_t> &data, uint64_t number, uint8_t bytes) {
    CHECK(bytes > 0);
    std::vector<uint8_t> tmp;
    for (uint8_t i = 0; i < bytes; i++) {
        tmp.push_back(number & 0xFF);
        number >>= 8;
    }
    std::reverse(tmp.begin(), tmp.end());
    data.insert(data.end(), tmp.begin(), tmp.end());
}

std::vector<Ptr<MyCell> > convert_to_my_cells(td::Ref<vm::Cell> root) {
    std::vector<Ptr<MyCell> > my_cells;

    std::unordered_map<const vm::Cell *, unsigned> pos;
    auto dfs = [&](auto &&self, td::Ref<vm::Cell> cell) -> void {
        if (pos.count(cell.get())) {
            return;
        }
        pos[cell.get()] = my_cells.size();
        auto my_cell = std::make_shared<MyCell>(cell);
        my_cells.push_back(my_cell);
        CHECK(my_cell->cnt_refs <= 4);
        for (unsigned i = 0; i < my_cell->cnt_refs; ++i) {
            self(self, my_cell->refs_as_cells[i]);
        }
    };

    dfs(dfs, root);

    for (const auto &cell: my_cells) {
        for (unsigned i = 0; i < cell->cnt_refs; ++i) {
            td::Ref<vm::Cell> ref = cell->refs_as_cells[i];
            cell->refs[i] = pos.at(ref.get());
        }
    }

    return my_cells;
}

td::BufferSlice serialize(td::Ref<vm::Cell> root) {
    auto my_cells = convert_to_my_cells(root);
    const unsigned n = my_cells.size();

    // reorder cells
    {
        std::vector<uint8_t> out_degree(n);
        std::vector<std::vector<unsigned> > who_refers(n);
        for (unsigned i = 0; i < n; i++) {
            out_degree[i] = my_cells[i]->cnt_refs;
            for (unsigned j = 0; j < my_cells[i]->cnt_refs; j++) {
                who_refers[my_cells[i]->refs[j]].push_back(i);
            }
        }

        auto cmp = [&](unsigned i, unsigned j) {
            if (out_degree[i] != out_degree[j]) {
                return out_degree[i] < out_degree[j];
            }
            if (who_refers[i].size() != who_refers[j].size()) {
                return who_refers[i].size() > who_refers[j].size();
            }
            return i < j;
        };

        std::set<unsigned, decltype(cmp)> cells(cmp);
        for (unsigned i = 0; i < n; i++) {
            cells.insert(i);
        }

        std::vector<unsigned> order;
        order.reserve(n);
        while (!cells.empty()) {
            auto it = cells.begin();
            unsigned i = *it;
            cells.erase(it);

            CHECK(out_degree[i] == 0);

            order.push_back(i);

            for (unsigned from: who_refers[i]) {
                cells.erase(from);
                CHECK(out_degree[from] > 0);
                --out_degree[from];
                cells.insert(from);
            }
        }

        // sort inside buckets with the same byte length
        auto cmp_desc = [&](unsigned i, unsigned j) {
            const auto d1_i = my_cells[i]->d1();
            const auto d1_j = my_cells[j]->d1();
            if (d1_i != d1_j) {
                return d1_i < d1_j;
            }

            const auto d2_i = my_cells[i]->d2();
            const auto d2_j = my_cells[j]->d2();
            if (d2_i != d2_j) {
                return d2_i < d2_j;
            }
            
            return std::make_pair(my_cells[i]->data, i) < std::make_pair(my_cells[j]->data, j);
        };

        uint8_t bytes = 1;
        for (unsigned l = 0, r = 0; l < n; l = r, ++bytes) {
            r = std::min(1u << (8 * bytes), n);

            // [l, r)
            std::sort(std::begin(order) + l, std::begin(order) + r, cmp_desc);
        }

        // update ref indexes
        std::vector<unsigned> pos(n);
        for (unsigned i = 0; i < n; i++) {
            pos[order[i]] = i;
        }

        for (auto &my_cell: my_cells) {
            for (unsigned i = 0; i < my_cell->cnt_refs; i++) {
                my_cell->refs[i] = pos[my_cell->refs[i]];
            }
        }

        {
            std::vector<Ptr<MyCell> > new_my_cells(n);
            for (unsigned i = 0; i < n; i++) {
                new_my_cells[pos[i]] = my_cells[i];
            }
            my_cells.swap(new_my_cells);
        }
    }

    std::basic_string<uint8_t> bytes;

    // the number of bytes needed to store a cell index and its references
    uint8_t bytes_for_index = 1;

    for (unsigned i = 0; i < n; ++i) {
        const auto my_cell = my_cells[i];
        if (i == (1u << (8 * bytes_for_index))) {
            ++bytes_for_index;
        }

        bytes.push_back(my_cell->d1());

        bytes.push_back(my_cell->d2());
        for (unsigned it = 0; it < my_cell->cnt_bits / 8; ++it) {
            bytes.push_back(my_cell->data[it]);
        }
        if (my_cell->cnt_bits % 8 != 0) {
            uint8_t pos = 7 - (my_cell->cnt_bits % 8);
            uint8_t last_byte = my_cell->data.at((my_cell->cnt_bits + 7) / 8 - 1);
            last_byte ^= (1u << pos);
            bytes.push_back(last_byte);
        }

        // up to 4 refs
        for (unsigned it = 0; it < my_cell->cnt_refs; ++it) {
            // big endian
            push_as_bytes(bytes, my_cell->refs[it], bytes_for_index);
        }
    }

    return td::BufferSlice(reinterpret_cast<const char *>(bytes.data()), bytes.size());
}

td::Ref<vm::Cell> deserialise(td::Slice buffer_slice) {
    std::vector<uint8_t> buffer(buffer_slice.data(), buffer_slice.data() + buffer_slice.size());

    std::vector<Ptr<MyCell> > my_cells;

    // build my_cells
    size_t ptr_buffer = 0;
    uint8_t bytes_for_index = 1;
    while (ptr_buffer < buffer.size()) {
        Ptr<MyCell> my_cell = std::make_shared<MyCell>();
        {
            size_t cur_cell_pos = my_cells.size();
            if (cur_cell_pos == (1u << (8 * bytes_for_index))) {
                ++bytes_for_index;
            }
        }

        // read first descriptor byte
        {
            uint8_t desc = buffer[ptr_buffer++];
            unsigned r = desc & 7;
            CHECK(r <= 4);
            bool s = (desc >> 3) & 1;

            my_cell->cnt_refs = r;
            my_cell->is_special = s;
        }

        // second descriptor byte and then the cell's data
        {
            uint8_t desc = buffer[ptr_buffer++];
            size_t cnt_bytes = desc >> 1;
            while (cnt_bytes > 0) {
                my_cell->data.push_back(buffer[ptr_buffer++]);
                --cnt_bytes;
            }
            my_cell->cnt_bits = my_cell->data.size() * 8;
            if (desc & 1) {
                uint8_t tail = buffer[ptr_buffer++];
                CHECK(tail != 0);
                int cnt_trailing_zeros = __builtin_ctz(tail);
                CHECK(cnt_trailing_zeros <= 6);
                unsigned cnt_bits_pref = 7 - cnt_trailing_zeros;
                tail ^= 1 << cnt_trailing_zeros;

                my_cell->data.push_back(tail);
                my_cell->cnt_bits += cnt_bits_pref;
            }
        }

        // read up to 4 refs
        {
            for (unsigned j = 0; j < my_cell->cnt_refs; j++) {
                // bytes_for_index bytes in big endian
                unsigned ref_id = 0;
                for (unsigned iter2 = 0; iter2 < bytes_for_index; ++iter2) {
                    ref_id = (ref_id << 8) ^ buffer[ptr_buffer++];
                }
                my_cell->refs[j] = ref_id;
            }
        }

        my_cells.push_back(my_cell);
        CHECK(ptr_buffer <= buffer.size());
    }
    const auto n = my_cells.size();
    CHECK(!my_cells.empty());

    // top sort
    std::vector<unsigned> order; {
        std::vector<char> was(n, false);
        auto dfs = [&](auto &&self, unsigned i) -> void {
            CHECK(!was[i]);
            was[i] = true;

            for (unsigned j = 0; j < my_cells[i]->cnt_refs; j++) {
                unsigned to = my_cells[i]->refs[j];
                if (!was[to]) {
                    self(self, to);
                }
            }

            order.push_back(i);
        };
        for (unsigned i = 0; i < n; ++i) {
            if (!was[i]) {
                dfs(dfs, i);
            }
        }
    }

    uint8_t bytes_for_index_max = len_in_bytes(my_cells.size());

    // build DAG of td::Ref<vm::Cell>
    std::vector<td::Ref<vm::Cell> > cells(n);
    for (unsigned i : order) {
        const auto &my_cell = my_cells.at(i);

        vm::CellBuilder cell_builder;

        // add refs
        std::vector<td::Ref<vm::Cell> > refs;
        for (unsigned j = 0; j < my_cell->cnt_refs; ++j) {
            unsigned ref_id = my_cell->refs[j];
            CHECK(len_in_bytes(ref_id) <= len_in_bytes(i));
            CHECK(cells[ref_id].get());
            cell_builder.store_ref(cells[ref_id]);
            refs.push_back(cells[ref_id]);
        }

        const auto cell_type = my_cell->get_type();
        std::basic_string<uint8_t> data = my_cell->data;;
        unsigned cnt_bits = my_cell->cnt_bits;
        if (cell_type == CellType::MerkleUpdate) {
            // hash0 (256), hash1, depth0 (8 * bytes_for_index), depth1
            CHECK(refs.size() == 2);
            CHECK(data.size() == 1);

            for (unsigned j = 0; j < 2; ++j) {
                const auto hash = refs[j]->get_hash(0);
                const auto slice = hash.as_slice();
                auto calc_hash = std::basic_string<uint8_t>(slice.data(), slice.data() + (256/8));
                // std::cout << "calc   first byte: " << std::bitset<8>(calc_hash[0]) << std::endl;
                // std::cout << "stored first byte: " << std::bitset<8>(data[1]) << std::endl;
                // CHECK(calc_hash == data.substr(1, 256/8));
                data += calc_hash;
            }

            for (unsigned j = 0; j < 2; ++j) {
                unsigned depth = refs[j]->get_depth(0);
                push_as_bytes(data, depth, bytes_for_index_max);
            }
            cnt_bits = data.size() * 8;
        }

        // add data
        cell_builder.store_bits(
            data.data(),
            cnt_bits
        );

        td::Ref<vm::DataCell> cell{};
        auto finalised = cell_builder.finalize_novm_nothrow(my_cell->is_special);
        cell = finalised.move_as_ok();
        CHECK(cell->is_special() == my_cell->is_special);

        cells[i] = cell;
    }

    return cells[order.back()];
}

std::string compress(const std::string &base64_data) {
    CHECK(!base64_data.empty());
    td::BufferSlice data(td::base64_decode(base64_data).move_as_ok());
    td::Ref<vm::Cell> root = vm::std_boc_deserialize(data).move_as_ok();

    td::BufferSlice serialized_my = serialize(root);
    // std::cout << "my serialised, number of bytes =  " << serialized.size() << std::endl;
    td::BufferSlice serialized_their = vm::std_boc_serialize(root, 0).move_as_ok();

    auto serialized = std::move(serialized_my);

    serialized = td::lz4_compress(serialized);

    return base64_encode(serialized);
}

std::string decompress(const std::string &base64_data) {
    CHECK(!base64_data.empty());
    td::BufferSlice data(td::base64_decode(base64_data).move_as_ok());

    data = td::lz4_decompress(data, 2 << 20).move_as_ok();

    td::Ref<vm::Cell> root = deserialise(data);

    // td::Ref<vm::Cell> root = vm::std_boc_deserialize(data).move_as_ok();

    td::BufferSlice serialised_properly = vm::std_boc_serialize(root, 31).move_as_ok();
    return base64_encode(serialised_properly);
}
};

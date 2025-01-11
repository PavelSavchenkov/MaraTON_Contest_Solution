#include <string>
#include "utils.h"
#include "utils_local.hpp"

void analyse(const std::string &block_path) {
    td::Ref<vm::Cell> root = read_from_filepath_to_root_cell(block_path);
}

int main() {
    const std::string pref = "/Users/pavel/Programming/MaraTON/ton-sample-tests/";
    const std::string files_names_file = pref + "file_list.txt";

    std::ifstream files_names(files_names_file);
    std::string line;
    while (files_names >> line) {
        if (line.empty() || line[0] != '1') {
            continue;
        }
        line = pref + line;

        // line = "/Users/pavel/Programming/MaraTON/ton-sample-tests/1-025.txt";

        analyse(line);
    }
}

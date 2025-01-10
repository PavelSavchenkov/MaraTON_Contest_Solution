import os

ROOT = "."
SOURCE = "main.cpp"

was = set()


def dfs(filename):
    assert filename not in was
    was.add(filename)

    file = open(os.path.join(ROOT, filename), "r")
    assert file is not None

    ans = ""
    for line in file.readlines():
        if len(ans) == 0:
            if "LOCAL" in line:
                return ""
        if "#pragma once" in line:
            continue
        if line.startswith("#include") and "<" not in line and "/" not in line:
            l = line.find("\"")
            l += 1
            r = line[l:].find("\"") + l
            to_filename = line[l:r]
            if to_filename not in was:
                ans += dfs(to_filename)
        else:
            compressed_line = ""
            in_str = False
            for i in range(len(line)):
                if line[i] == '\"':
                    in_str = True
                if line[i] == '/' and i + 1 < len(line) and line[i + 1] == '/' and not in_str:
                    break
                else:
                    compressed_line += line[i]
            ans += compressed_line
    return ans


ans = dfs(SOURCE)
with open(os.path.join(ROOT, "to_submit.cpp"), "w") as f:
    f.write(ans)

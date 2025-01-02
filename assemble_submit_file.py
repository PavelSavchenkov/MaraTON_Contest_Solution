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
            ans += line
    return ans


ans = dfs(SOURCE)
with open(os.path.join(ROOT, "to_submit.cpp"), "w") as f:
    f.write(ans)

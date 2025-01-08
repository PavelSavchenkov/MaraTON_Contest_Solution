import base64
import binascii
import os
import subprocess
import time
from typing import Optional
from colorama import Fore, Style, init
import pickle
import argparse


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--exec', type=str)
    parser.add_argument('--baseline', type=str, default=None, required=False)
    return parser.parse_args()


def get_base64_size(s: str) -> Optional[int]:
    try:
        decoded = base64.b64decode(s, validate=True)
    except binascii.Error as e:
        # print(f"Base64 decoding error: {e}")
        return None
    return len(decoded)


args = parse_args()
solution_path = args.exec
baseline_path = args.baseline
baseline = None
if baseline_path is not None:
    baseline = pickle.load(open(baseline_path, 'rb'))

init(autoreset=True)
tests_dir = "ton-sample-tests"
results = dict()

test_files = [fname for fname in os.listdir(tests_dir) if fname.startswith("1")]
test_files.sort()
n_tests = len(test_files)
if n_tests == 0:
    print("Error: no tests")
    exit(2)

n_ok = 0
total_points = 0

name_width = max(max(len(s) for s in test_files), 4)

print(f"{Fore.YELLOW}====================== Running {n_tests} tests ======================{Style.RESET_ALL}")
print(f"{Fore.YELLOW}Idx  {"Name":<{name_width}} {"Size":>7}   Compressed     ", end="")
if baseline is not None:
    print(f"BCompressed     ", end="")
print(f"Points         ", end="")
if baseline is not None:
    print(f"BPoints     ", end="")
print(f"Time {Style.RESET_ALL}")

for i, test_file in enumerate(test_files):
    i += 1
    with open(os.path.join(tests_dir, test_file), "r") as f:
        original_block = f.read().split()[1]
    original_size = get_base64_size(original_block)
    assert original_size is not None
    assert original_size <= 2 ** 21
    print(f"{Fore.YELLOW}{i:>4}{Style.RESET_ALL}",
          f"{test_file:<{name_width}}",
          f"{Fore.CYAN}{original_size:>7}{Style.RESET_ALL}",
          end="   ")

    start_time = time.time()
    try:
        result = subprocess.run("./" + solution_path, input="compress\n" + original_block, text=True,
                                capture_output=True,
                                timeout=100.0)
    except subprocess.TimeoutExpired:
        print(f"{Fore.RED}TL timeout expired{Style.RESET_ALL}")
        continue
    if result.returncode != 0:
        print(f"{Fore.RED}RE exitcode={result.returncode}{Style.RESET_ALL}")
        continue
    solution_time = time.time() - start_time
    compressed_block = result.stdout.strip()
    compressed_size = get_base64_size(compressed_block)
    if compressed_size is None:
        print(f"{Fore.RED}PE invalid base64{Style.RESET_ALL}")
        # print(f"compressed_block={compressed_block}")
        # exit(0)
        continue
    if compressed_size > 2 ** 21:
        print(f"{Fore.RED}PE too big output ({compressed_size}){Style.RESET_ALL}")
        continue
    compression_color = Fore.CYAN
    if baseline is not None and baseline[test_file]["compressed_size"] < compressed_size:
        compression_color = Fore.RED
    print(f"{Fore.GREEN}OK {compression_color}{compressed_size:>7}{Style.RESET_ALL}", end="    ")
    if baseline is not None:
        baseline_color = Fore.RED
        if compression_color == Fore.RED:
            baseline_color = Fore.CYAN
        print(f"{baseline_color}{baseline[test_file]["compressed_size"]:>7}{Style.RESET_ALL}", end="          ")

    try:
        result = subprocess.run("./" + solution_path, input="decompress\n" + compressed_block, text=True,
                                capture_output=True,
                                timeout=2.0)
    except subprocess.TimeoutExpired:
        print(f"{Fore.RED}TL timeout expired{Style.RESET_ALL}")
        continue
    if result.returncode != 0:
        print(f"{Fore.RED}RE exitcode={result.returncode}{Style.RESET_ALL}")
        continue
    decompressed_block = result.stdout.strip()
    if decompressed_block != original_block:
        print(f"{Fore.RED}WA wrong decompressed block{Style.RESET_ALL}")
        continue

    points = 1000 * (2 * original_size) / (original_size + compressed_size)
    points_color = Fore.CYAN
    if baseline is not None and baseline[test_file]["points"] > points:
        points_color = Fore.RED
    print(f"{Fore.GREEN}OK{Style.RESET_ALL} {points_color}{points:9.3f}", end="  ")
    if baseline is not None:
        baseline_color = Fore.RED
        if points_color == Fore.RED:
            baseline_color = Fore.CYAN
        print(f"{baseline_color}{baseline[test_file]["points"]:9.3f}", end=" ")
    print(f"{Fore.CYAN}{solution_time:9.3f}{Style.RESET_ALL}")
    n_ok += 1
    total_points += points

    results[test_file] = {"points": points, "compressed_size": compressed_size}

if n_ok == n_tests:
    passed_color = Fore.GREEN
else:
    passed_color = Fore.RED
print(f"{Fore.YELLOW}Passed tests:   {passed_color}{n_ok}/{n_tests}{Style.RESET_ALL}")
print(f"{Fore.YELLOW}Average points: {Fore.CYAN}{total_points / n_tests:.3f}{Style.RESET_ALL}")

points_color = Fore.CYAN
if baseline is not None and baseline["total"] > total_points:
    points_color = Fore.RED
print(f"{Fore.YELLOW}Total points:   {points_color}{total_points:.3f}{Style.RESET_ALL}")
if baseline is not None:
    baseline_color = Fore.RED
    if points_color == Fore.RED:
        baseline_color = Fore.CYAN
    print(f"{Fore.YELLOW}BTotal points:  {baseline_color}{baseline["total"]:.3f}{Style.RESET_ALL}")
results["total"] = total_points
with open("points/last.pickle", "wb") as file:
    pickle.dump(results, file)

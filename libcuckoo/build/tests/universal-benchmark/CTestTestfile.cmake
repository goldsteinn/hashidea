# CMake generated Testfile for 
# Source directory: /home/noah/programs/hashidea/other_hash/libcuckoo/tests/universal-benchmark
# Build directory: /home/noah/programs/hashidea/other_hash/libcuckoo/build/tests/universal-benchmark
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(pure_read "/home/noah/programs/hashidea/other_hash/libcuckoo/build/tests/universal-benchmark/universal_benchmark" "--reads" "100" "--prefill" "75" "--total-ops" "500" "--initial-capacity" "23")
add_test(pure_insert "/home/noah/programs/hashidea/other_hash/libcuckoo/build/tests/universal-benchmark/universal_benchmark" "--inserts" "100" "--total-ops" "75" "--initial-capacity" "23")
add_test(pure_erase "/home/noah/programs/hashidea/other_hash/libcuckoo/build/tests/universal-benchmark/universal_benchmark" "--erases" "100" "--prefill" "75" "--total-ops" "75" "--initial-capacity" "23")
add_test(pure_update "/home/noah/programs/hashidea/other_hash/libcuckoo/build/tests/universal-benchmark/universal_benchmark" "--updates" "100" "--prefill" "75" "--total-ops" "500" "--initial-capacity" "23")
add_test(pure_upsert "/home/noah/programs/hashidea/other_hash/libcuckoo/build/tests/universal-benchmark/universal_benchmark" "--upserts" "100" "--prefill" "25" "--total-ops" "200" "--initial-capacity" "23")
add_test(insert_expansion "/home/noah/programs/hashidea/other_hash/libcuckoo/build/tests/universal-benchmark/universal_benchmark" "--inserts" "100" "--initial-capacity" "4" "--total-ops" "13107200")
add_test(read_insert_expansion "/home/noah/programs/hashidea/other_hash/libcuckoo/build/tests/universal-benchmark/universal_benchmark" "--reads" "80" "--inserts" "20" "--initial-capacity" "10" "--total-ops" "4096000")

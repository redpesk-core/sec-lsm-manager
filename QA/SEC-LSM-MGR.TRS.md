# Test results of redpesk-core/sec-lsm-manager

.VERSION: 2.6.1

.AUTHOR: José Bollo [IoT.bzh]

.AUDIENCE: ENGINEERING

.DIFFUSION: PUBLIC

.git-id($Id$)

## Results of units testing

- Tested version: 2.6.1
- Operator: José Bollo
- Date: 2023-dec-06
- Platform: Debian GNU/Linux trixie/sid
- Arch: amd64

### Methodology

In a development environment where cynagora is running.
Units testing is performed by issuing the following commands

```
mkdir build
cd build
cmake .. -DSIMULATE_SMACK=YES -DWITH_SMACK=YES
make
make test
```

It runs the test whose sources are in the directory `src/tests`.

### produced artifacts

The produced TAP file is `build/src/tests/test-smack.tap`. Its content is:


```
ok 1 - <ROOT>/src/tests/test-prot.c:prot:test_prot_create: Passed
ok 2 - <ROOT>/src/tests/test-prot.c:prot:test_prot_put_field_by_field: Passed
ok 3 - <ROOT>/src/tests/test-prot.c:prot:test_prot_put_all_fields: Passed
ok 4 - <ROOT>/src/tests/test-prot.c:prot:test_prot_put_all: Passed
ok 5 - <ROOT>/src/tests/test-prot.c:prot:test_prot_read: Passed
ok 6 - <ROOT>/src/tests/test-prot.c:prot:test_prot_write_read: Passed
ok 7 - <ROOT>/src/tests/test-paths.c:paths:test_init_path_set: Passed
ok 8 - <ROOT>/src/tests/test-paths.c:paths:test_free_path_set: Passed
ok 9 - <ROOT>/src/tests/test-paths.c:paths:test_path_set_add_path: Passed
ok 10 - <ROOT>/src/tests/test-paths.c:paths:test_valid_path_type: Passed
ok 11 - <ROOT>/src/tests/test-paths.c:paths:test_get_path_type: Passed
ok 12 - <ROOT>/src/tests/test-paths.c:paths:test_get_path_type_string: Passed
ok 13 - <ROOT>/src/tests/test-permissions.c:permissions:test_init_permission_set: Passed
ok 14 - <ROOT>/src/tests/test-permissions.c:permissions:test_free_permission_set: Passed
ok 15 - <ROOT>/src/tests/test-permissions.c:permissions:test_permission_set_add_permission: Passed
ok 16 - <ROOT>/src/tests/test-context.c:context:test_init_context: Passed
ok 17 - <ROOT>/src/tests/test-context.c:context:test_create_context: Passed
ok 18 - <ROOT>/src/tests/test-context.c:context:test_context_set_id: Passed
ok 19 - <ROOT>/src/tests/test-context.c:context:test_context_add_permission: Passed
ok 20 - <ROOT>/src/tests/test-context.c:context:test_context_add_path: Passed
ok 21 - <ROOT>/src/tests/test-context.c:context:test_free_context: Passed
ok 22 - <ROOT>/src/tests/test-context.c:context:test_destroy_context: Passed
ok 23 - <ROOT>/src/tests/test-utils.c:utils:test_check_file_exists: Passed
ok 24 - <ROOT>/src/tests/test-utils.c:utils:test_check_dir: Passed
ok 25 - <ROOT>/src/tests/test-utils.c:utils:test_check_executable: Passed
ok 26 - <ROOT>/src/tests/test-utils.c:utils:test_remove_file: Passed
ok 27 - <ROOT>/src/tests/test-utils.c:utils:test_path_std: Passed
ok 28 - <ROOT>/src/tests/test-plugs.c:plug:test_plugset_init: Passed
ok 29 - <ROOT>/src/tests/test-plugs.c:plug:test_plugset_deinit: Passed
ok 30 - <ROOT>/src/tests/test-plugs.c:plug:test_plugset_add: Passed
ok 31 - <ROOT>/src/tests/test-cynagora.c:cynagora:test_cynagora_set_policies: Passed
ok 32 - <ROOT>/src/tests/test-cynagora.c:cynagora:test_cynagora_drop_policies: Passed
1..32
```

## Results of integration test

- Tested version: 2.6.1
- Operator: José Bollo
- Date: 2023-dec-06
- Platform: redpesk Linux arz_1_2 LTS
- Arch: amd64

### Methodology

1. On QEMU redpesk image, run the tests script `TESTS/test1/do-test1.sh`
2. Get the results in the repository in `TESTS/test1/results`
3. Check the results against the references and update references when necessary
4. Run the script `check-test1.sh` it produces synthetic TAP file

### produced artifacts

The TAP file is saved in `TESTS/test1/test1.tap`. Here is its content:

```
TAP version 14
ok 1 - SEC-LSM-MGR.HTC-T-CHE-IDE-VAL
ok 2 - SEC-LSM-MGR.HTC-T-CHE-PAT-EXI-PAT-PRO
ok 3 - SEC-LSM-MGR.HTC-T-CHE-PAT-TYP-VAL-PAT-PRO
ok 4 - SEC-LSM-MGR.HTC-T-CHE-PER-VAL
ok 5 - SEC-LSM-MGR.HTC-T-CHE-PLU-VAL
ok 6 - SEC-LSM-MGR.HTC-T-CLE-CON
ok 7 - SEC-LSM-MGR.HTC-T-ERR-STA
ok 8 - SEC-LSM-MGR.HTC-T-NO-PAT-DUP
ok 9 - SEC-LSM-MGR.HTC-T-NO-PER-DUP
ok 10 - SEC-LSM-MGR.HTC-T-NO-SEC-ID-PRO
ok 11 - SEC-LSM-MGR.HTC-T-QUE-CON
ok 12 - SEC-LSM-MGR.HTC-T-SER-INS
ok 13 - SEC-LSM-MGR.HTC-T-SER-SOC-PRO
ok 14 - SEC-LSM-MGR.HTC-T-SER-STA-AUT
ok 15 - SEC-LSM-MGR.HTC-T-SET-ID-PRO
ok 16 - SEC-LSM-MGR.HTC-T-SET-PAT-PRO
ok 17 - SEC-LSM-MGR.HTC-T-SET-PER-PRO
ok 18 - SEC-LSM-MGR.HTC-T-SET-PLU-PRO
1..18
```

## Results of coverage measurement

- Tested version: 2.6.1
- Operator: José Bollo
- Date: 2023-dec-06
- Platform: Debian GNU/Linux trixie/sid
- Arch: amd64

### Methodology

In a development environment where cynagora is launched on
command line, the coverage test is performed by issuing the
following commands

```
mkdir build
cd build
cmake .. -DSIMULATE_SMACK=YES -DWITH_SMACK=YES -DCOVERAGE=YES
make install
cd ../TESTS/cov1
./do-cov1.sh
```

The coverage test is conducted by the BASH script `TESTS/cov1/do-cov1.sh`.

### produced artifacts

It produces the folowing artifacts:

- `TESTS/cov1/coverage.txt`: full coverage report in text
- `TESTS/cov1/coverage-summary.txt`: summary of the previous one (missing line removed), stored in Git
- `TESTS/cov1/coverage`: the full HTML report with details (see `TESTS/cov1/coverage/index.html`)
- `TESTS/cov1/coverage-summary.html`: HTML summary of the coverage report


The summary report `TESTS/cov1/coverage-summary.txt` for version
tagged 2.6.1 (SHA d8e176e) is:

```
----------------------------------------------------------------
                           GCC Code Coverage Report
Directory: ../../src
----------------------------------------------------------------
File                                       Lines    Exec  Cover 
----------------------------------------------------------------
path-utils.c                                  32      32   100%
log.c                                         19      19   100%
utf8-utils.c                                  11      11   100%
context/plugs.c                               41      41   100%
protocol/sec-lsm-manager-protocol.c            6       6   100%
lsm-smack/xattr-smack.h                        6       6   100%
context/context.c                            174     168    96% 
context/paths.c                               53      49    92% 
protocol/prot.c                              209     190    90% 
main-sec-lsm-managerd.c                      234     212    90% 
protocol/pollitem.c                           18      16    88% 
context/permissions.c                         42      37    88% 
permission/cynagora-interface.c               66      57    86% 
protocol/sec-lsm-manager.c                   243     207    85% 
protocol/client.c                            269     224    83% 
action/action.c                               74      61    82% 
xattr-utils.c                                 16      13    81% 
lsm-smack/smack.c                            150     116    77% 
templating/template.c                         65      50    76% 
lsm-smack/smack-template.c                    99      74    74% 
file-utils.c                                  88      61    69% 
protocol/sec-lsm-manager-server.c            141      95    67% 
protocol/socket.c                            114      46    40% 
templating/mustach.c                         228      91    39% 
----------------------------------------------------------------
TOTAL                                       2398    1882    78%
----------------------------------------------------------------
```

The poor result for the file `templating/mustach.c` can be explained
by the fact that this module is extracted from a generic solution
(https://gitlab.com/jobol/mustach) and that only a little part of
its provided features is used.

The poor result for the file `protocol/socket.c` can be explained
by the fact that this module is copied from other internal projects
and that only part of its feature is used. Though, the part related
to systemd integration should be covered but is not covered.

For the remaining parts, the missing coverage is mostly related to
code doing:

- processing of exceptional errors
- processing of networking contention

## Results of fuzzing test

- Tested version: 2.6.0
- Operator: José Bollo
- Date: 2023-nov-29
- Platform: Debian GNU/Linux trixie/sid
- Arch: amd64

### Methodology

In a development environment where AFL++ is available,
fuzzing is prepared by issuing the following command

```
./fuzz.sh
```

It produces in the directory `afl-build` the required programs.
At the end, it prompts the commands for running 3 fuzzing in parallel:
one standard fuzzing, one with ASAN instrumentation and one with UBSAN
instrumentation.

In separate terminals, run the given commands, beginning with the master
(option `-M`) one.

### produced artifacts

The produced artifacts are in directories

- `afl/OUTPUTS/asan/`
- `afl/OUTPUTS/std/`
- `afl/OUTPUTS/ubsan/`

Here copies of saved stats.

### afl/fuzzer\_stats-1

```
start_time        : 1698922143
last_update       : 1699287634
run_time          : 365490
fuzzer_pid        : 2825665
cycles_done       : 581
cycles_wo_finds   : 183
time_wo_finds     : 114037
execs_done        : 435407831
execs_per_sec     : 1191.30
execs_ps_last_min : 1128.83
corpus_count      : 914
corpus_favored    : 78
corpus_found      : 905
corpus_imported   : 0
corpus_variable   : 0
max_depth         : 14
cur_item          : 136
pending_favs      : 0
pending_total     : 2
stability         : 100.00%
bitmap_cvg        : 47.72%
saved_crashes     : 0
saved_hangs       : 0
last_find         : 1699173596
last_crash        : 0
last_hang         : 0
execs_since_crash : 435407831
exec_timeout      : 20
slowest_exec_ms   : 0
peak_rss_mb       : 3
cpu_affinity      : 0
edges_found       : 554
total_edges       : 1161
var_byte_count    : 0
havoc_expansion   : 5
auto_dict_entries : 0
testcache_size    : 1582102
testcache_count   : 914
testcache_evict   : 0
afl_banner        : ...sts/slmc-test-simcyn-simsma
afl_version       : ++4.08c
target_mode       : shmem_testcase default
command_line      : afl-fuzz -i INPUTS -o OUTPUTS -- <ROOT>/afl-build/src/tests/slmc-test-simcyn-simsma
```


### afl/fuzzer\_stats-2.std

```
start_time        : 1700751492
last_update       : 1700864588
run_time          : 113095
fuzzer_pid        : 2569601
cycles_done       : 651
cycles_wo_finds   : 32
time_wo_finds     : 9112
execs_done        : 27955950
execs_per_sec     : 247.19
execs_ps_last_min : 1477.86
corpus_count      : 964
corpus_favored    : 75
corpus_found      : 8
corpus_imported   : 2
corpus_variable   : 0
max_depth         : 2
cur_item          : 487
pending_favs      : 0
pending_total     : 0
stability         : 100.00%
bitmap_cvg        : 43.03%
saved_crashes     : 0
saved_hangs       : 0
last_find         : 1700859164
last_crash        : 0
last_hang         : 0
execs_since_crash : 27955950
exec_timeout      : 40
slowest_exec_ms   : 0
peak_rss_mb       : 3
cpu_affinity      : 0
edges_found       : 525
total_edges       : 1220
var_byte_count    : 0
havoc_expansion   : 5
auto_dict_entries : 0
testcache_size    : 4326356
testcache_count   : 964
testcache_evict   : 0
afl_banner        : ...sts/slmc-test-simcyn-simsma.std
afl_version       : ++4.08c
target_mode       : shmem_testcase default
command_line      : afl-fuzz -P explore -M std -i - -o OUTPUTS -- <ROOT>/afl-build/src/tests/slmc-test-simcyn-simsma.std
```

### afl/fuzzer\_stats-2.asan

```
start_time        : 1700753406
last_update       : 1700864591
run_time          : 111185
fuzzer_pid        : 2569741
cycles_done       : 55
cycles_wo_finds   : 0
time_wo_finds     : 3729
execs_done        : 17199736
execs_per_sec     : 154.69
execs_ps_last_min : 637.05
corpus_count      : 940
corpus_favored    : 84
corpus_found      : 28
corpus_imported   : 4
corpus_variable   : 0
max_depth         : 2
cur_item          : 463
pending_favs      : 0
pending_total     : 513
stability         : 100.00%
bitmap_cvg        : 41.74%
saved_crashes     : 0
saved_hangs       : 0
last_find         : 1700864107
last_crash        : 0
last_hang         : 0
execs_since_crash : 17199736
exec_timeout      : 40
slowest_exec_ms   : 0
peak_rss_mb       : 12
cpu_affinity      : 1
edges_found       : 518
total_edges       : 1241
var_byte_count    : 0
havoc_expansion   : 1
auto_dict_entries : 0
testcache_size    : 3311252
testcache_count   : 940
testcache_evict   : 0
afl_banner        : ...s/slmc-test-simcyn-simsma.asan
afl_version       : ++4.08c
target_mode       : shmem_testcase default
command_line      : afl-fuzz -P explore -S asan -i - -o OUTPUTS -- <ROOT>/afl-build/src/tests/slmc-test-simcyn-simsma.asan
```

### afl/fuzzer\_stats-2.ubsan

```
start_time        : 1700772919
last_update       : 1700864593
run_time          : 91674
fuzzer_pid        : 2570064
cycles_done       : 174
cycles_wo_finds   : 2
time_wo_finds     : 8743
execs_done        : 27639647
execs_per_sec     : 301.50
execs_ps_last_min : 830.59
corpus_count      : 536
corpus_favored    : 63
corpus_found      : 24
corpus_imported   : 4
corpus_variable   : 0
max_depth         : 2
cur_item          : 436
pending_favs      : 0
pending_total     : 246
stability         : 100.00%
bitmap_cvg        : 24.99%
saved_crashes     : 0
saved_hangs       : 0
last_find         : 1700859561
last_crash        : 0
last_hang         : 0
execs_since_crash : 27639647
exec_timeout      : 40
slowest_exec_ms   : 0
peak_rss_mb       : 3
cpu_affinity      : 2
edges_found       : 558
total_edges       : 2233
var_byte_count    : 0
havoc_expansion   : 5
auto_dict_entries : 0
testcache_size    : 2692309
testcache_count   : 536
testcache_evict   : 0
afl_banner        : ...slmc-test-simcyn-simsma.ubsan
afl_version       : ++4.08c
target_mode       : shmem_testcase default
command_line      : afl-fuzz -P explore -S ubsan -i - -o OUTPUTS -- <ROOT>/afl-build/src/tests/slmc-test-simcyn-simsma.ubsan
```


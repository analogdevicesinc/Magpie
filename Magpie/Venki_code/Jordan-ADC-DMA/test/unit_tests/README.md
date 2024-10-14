# Magpie Main Microcontroller Unit Tests

## Brief

- This directory is a sandbox for trying out different unit-testing workflows
- Gtest and Gmock are used to test isolated functions from the application code
- Not all code is easy to test, for example code that relies heavily on the peripherals of the MAX32666
- The goal here is to keep as much code as possible microcontroller agnostic so that it can be tested, but it is not expected to test every firmware module

## Setup/installation

### Windows

#### Setup step (1) Install [Cygwin](https://www.cygwin.com/) by downloading and running the installer.

Follow the instructions below (taken from [Klein embedded](https://kleinembedded.com/unit-testing-in-stm32cubeide-with-googletest/)):

When you get to the Select Packages page during installation, select Full in the View dropdown menu to show all packages. Then select the following packages for installation by selecting the latest version in the dropdown menu where it says “Skip”:

- gcc-code
- gcc-g++
- gdb (choose v9.2-1, newer versions do not work with Eclipse)
- binutils
- make
- cmake
- python3


#### Setup step (2) Install Google Test

Download the newest release of GoogleTest (v. 1.14.0 at the moment of writing this) from the [Github repository](https://github.com/google/googletest). Unpack the zip-archive. Then open a Cygwin Terminal, browse to the download location and compile the library, e.g.:
```
$ cd /cygdrive/c/Users/YourName/Downloads/googletest-release-1.14.0
$ mkdir build && cd build
$ cmake ..
$ make
```

If everything goes smoothly, the library files should now be located in the lib folder within the current working directory, while the header files are located in the googletest/include and googlemock/include folders in the parent directory.

To install both header and library files to the Cygwin directory (i.e. `C:\cygwin\usr\local\include` and `C:\cygwin\usr\local\lib`), also run:
```
$ make install
```

If the compiler complains that it can't find `-lgtest`, `-lgtest_main` or `-lgmock`, copy/move the libraries from `C:\cygwin\usr\local\lib` to `C\cygwin\usr\your-pc-architecture\lib`.

The library files to move are `libgmock.a`, `libgmock_main.a`, `libgtest.a`, and `libgtest_main.a`.

### MacOS/Linux

- If anyone on the team is using Mac or Linux to run the unit tests, please fill in this section!

## Quirks/limitations

- The included Makefile is  _very_  rudimentary at the moment, it contains just barely enough code to get some tests running, with lots of yucky copy-paste
- We will need to create some test running code so that it's fast and easy to run the tests
    - It would be ideal if this integrated with the VScode test runner so we could run all tests with a button click
- Not very many modules are tested yet

## To run the tests

- Navigate to this directory
- `$ make`
    - The default command prints only a brief summary plus any failing tests
- `$ make verbose`
    - The verbose version prints out info about all the tests, not just those that fail
- `$ make report`
    - This option generates an XML file of the test results in this directory
- `$ make clean`
    - Delete any test executable files and build artifacts

## Expected sample output (all passing, verbose mode off)

```
[==========] 23 tests from 2 test suites ran. (0 ms total)
[  PASSED  ] 23 tests.
```

## Expected sample output (all passing, verbose mode on)


```
[==========] Running 23 tests from 2 test suites.
[----------] Global test environment set-up.
[----------] 14 tests from TestWavHeader
[ RUN      ] TestWavHeader.header_length_is_44_bytes
[       OK ] TestWavHeader.header_length_is_44_bytes (0 ms)
[ RUN      ] TestWavHeader.string_RIFF_is_at_the_start
[       OK ] TestWavHeader.string_RIFF_is_at_the_start (0 ms)
[ RUN      ] TestWavHeader.eight_is_subtracted_from_file_len
[       OK ] TestWavHeader.eight_is_subtracted_from_file_len (0 ms)
[ RUN      ] TestWavHeader.string_WAVE_is_in_the_right_spot
[       OK ] TestWavHeader.string_WAVE_is_in_the_right_spot (0 ms)
[ RUN      ] TestWavHeader.string_fmt_is_in_the_right_spot
[       OK ] TestWavHeader.string_fmt_is_in_the_right_spot (0 ms)
[ RUN      ] TestWavHeader.fmt_chunk_size_is_always_16
[       OK ] TestWavHeader.fmt_chunk_size_is_always_16 (0 ms)
[ RUN      ] TestWavHeader.fmt_tag_is_always_PCM
[       OK ] TestWavHeader.fmt_tag_is_always_PCM (0 ms)
[ RUN      ] TestWavHeader.num_channels_can_be_set
[       OK ] TestWavHeader.num_channels_can_be_set (0 ms)
[ RUN      ] TestWavHeader.sample_rate_can_be_set
[       OK ] TestWavHeader.sample_rate_can_be_set (0 ms)
[ RUN      ] TestWavHeader.bytes_per_sec_are_calculated_and_set
[       OK ] TestWavHeader.bytes_per_sec_are_calculated_and_set (0 ms)
[ RUN      ] TestWavHeader.bytes_per_block_are_calculated_and_set
[       OK ] TestWavHeader.bytes_per_block_are_calculated_and_set (0 ms)
[ RUN      ] TestWavHeader.bits_per_sample_are_set
[       OK ] TestWavHeader.bits_per_sample_are_set (0 ms)
[ RUN      ] TestWavHeader.string_data_is_in_the_right_spot
[       OK ] TestWavHeader.string_data_is_in_the_right_spot (0 ms)
[ RUN      ] TestWavHeader.data_size_does_not_include_header_len
[       OK ] TestWavHeader.data_size_does_not_include_header_len (0 ms)
[----------] 14 tests from TestWavHeader (0 ms total)

[----------] 9 tests from TestDataConverters
[ RUN      ] TestDataConverters.i24_swap_endianness_smallest_chunk_check_all_bytes
[       OK ] TestDataConverters.i24_swap_endianness_smallest_chunk_check_all_bytes (0 ms)
[ RUN      ] TestDataConverters.i24_swap_endianness_check_sample_in_the_middle
[       OK ] TestDataConverters.i24_swap_endianness_check_sample_in_the_middle (0 ms)
[ RUN      ] TestDataConverters.i24_swap_endianness_should_not_go_past_array_end
[       OK ] TestDataConverters.i24_swap_endianness_should_not_go_past_array_end (0 ms)
[ RUN      ] TestDataConverters.i24_to_q31_smallest_chunk_check_all_bytes
[       OK ] TestDataConverters.i24_to_q31_smallest_chunk_check_all_bytes (0 ms)
[ RUN      ] TestDataConverters.i24_to_q31_check_sample_in_the_middle
[       OK ] TestDataConverters.i24_to_q31_check_sample_in_the_middle (0 ms)
[ RUN      ] TestDataConverters.i24_to_q31_should_not_go_past_array_end
[       OK ] TestDataConverters.i24_to_q31_should_not_go_past_array_end (0 ms)
[ RUN      ] TestDataConverters.q31_to_i24_smallest_chunk_check_all_bytes
[       OK ] TestDataConverters.q31_to_i24_smallest_chunk_check_all_bytes (0 ms)
[ RUN      ] TestDataConverters.q31_to_i24_check_sample_in_the_middle
[       OK ] TestDataConverters.q31_to_i24_check_sample_in_the_middle (0 ms)
[ RUN      ] TestDataConverters.q31_to_i24_should_not_go_past_array_end
[       OK ] TestDataConverters.q31_to_i24_should_not_go_past_array_end (0 ms)
[----------] 9 tests from TestDataConverters (0 ms total)

[----------] Global test environment tear-down
[==========] 23 tests from 2 test suites ran. (0 ms total)
[  PASSED  ] 23 tests.

```

## Expected sample output (with failure, verbose mode off)

```
test_wav_header.cpp:105: Failure
Expected equality of these values:
  sample_rate
    Which is: 16000
  16001

[  FAILED  ] TestWavHeader.sample_rate_can_be_set (0 ms)
[==========] 23 tests from 2 test suites ran. (0 ms total)
[  PASSED  ] 22 tests.
```

## Expected sample output (with failure, verbose mode on)

```
[==========] Running 23 tests from 2 test suites.
[----------] Global test environment set-up.
[----------] 14 tests from TestWavHeader
[ RUN      ] TestWavHeader.header_length_is_44_bytes
[       OK ] TestWavHeader.header_length_is_44_bytes (0 ms)
[ RUN      ] TestWavHeader.string_RIFF_is_at_the_start
[       OK ] TestWavHeader.string_RIFF_is_at_the_start (0 ms)
[ RUN      ] TestWavHeader.eight_is_subtracted_from_file_len
[       OK ] TestWavHeader.eight_is_subtracted_from_file_len (0 ms)
[ RUN      ] TestWavHeader.string_WAVE_is_in_the_right_spot
[       OK ] TestWavHeader.string_WAVE_is_in_the_right_spot (0 ms)
[ RUN      ] TestWavHeader.string_fmt_is_in_the_right_spot
[       OK ] TestWavHeader.string_fmt_is_in_the_right_spot (0 ms)
[ RUN      ] TestWavHeader.fmt_chunk_size_is_always_16
[       OK ] TestWavHeader.fmt_chunk_size_is_always_16 (0 ms)
[ RUN      ] TestWavHeader.fmt_tag_is_always_PCM
[       OK ] TestWavHeader.fmt_tag_is_always_PCM (0 ms)
[ RUN      ] TestWavHeader.num_channels_can_be_set
[       OK ] TestWavHeader.num_channels_can_be_set (0 ms)
[ RUN      ] TestWavHeader.sample_rate_can_be_set
test_wav_header.cpp:105: Failure
Expected equality of these values:
  sample_rate
    Which is: 16000
  16001

[  FAILED  ] TestWavHeader.sample_rate_can_be_set (0 ms)
[ RUN      ] TestWavHeader.bytes_per_sec_are_calculated_and_set
[       OK ] TestWavHeader.bytes_per_sec_are_calculated_and_set (0 ms)
[ RUN      ] TestWavHeader.bytes_per_block_are_calculated_and_set
[       OK ] TestWavHeader.bytes_per_block_are_calculated_and_set (0 ms)
[ RUN      ] TestWavHeader.bits_per_sample_are_set
[       OK ] TestWavHeader.bits_per_sample_are_set (0 ms)
[ RUN      ] TestWavHeader.string_data_is_in_the_right_spot
[       OK ] TestWavHeader.string_data_is_in_the_right_spot (0 ms)
[ RUN      ] TestWavHeader.data_size_does_not_include_header_len
[       OK ] TestWavHeader.data_size_does_not_include_header_len (0 ms)
[----------] 14 tests from TestWavHeader (0 ms total)

[----------] 9 tests from TestDataConverters
[ RUN      ] TestDataConverters.i24_swap_endianness_smallest_chunk_check_all_bytes
[       OK ] TestDataConverters.i24_swap_endianness_smallest_chunk_check_all_bytes (0 ms)
[ RUN      ] TestDataConverters.i24_swap_endianness_check_sample_in_the_middle
[       OK ] TestDataConverters.i24_swap_endianness_check_sample_in_the_middle (0 ms)
[ RUN      ] TestDataConverters.i24_swap_endianness_should_not_go_past_array_end
[       OK ] TestDataConverters.i24_swap_endianness_should_not_go_past_array_end (0 ms)
[ RUN      ] TestDataConverters.i24_to_q31_smallest_chunk_check_all_bytes
[       OK ] TestDataConverters.i24_to_q31_smallest_chunk_check_all_bytes (0 ms)
[ RUN      ] TestDataConverters.i24_to_q31_check_sample_in_the_middle
[       OK ] TestDataConverters.i24_to_q31_check_sample_in_the_middle (0 ms)
[ RUN      ] TestDataConverters.i24_to_q31_should_not_go_past_array_end
[       OK ] TestDataConverters.i24_to_q31_should_not_go_past_array_end (0 ms)
[ RUN      ] TestDataConverters.q31_to_i24_smallest_chunk_check_all_bytes
[       OK ] TestDataConverters.q31_to_i24_smallest_chunk_check_all_bytes (0 ms)
[ RUN      ] TestDataConverters.q31_to_i24_check_sample_in_the_middle
[       OK ] TestDataConverters.q31_to_i24_check_sample_in_the_middle (0 ms)
[ RUN      ] TestDataConverters.q31_to_i24_should_not_go_past_array_end
[       OK ] TestDataConverters.q31_to_i24_should_not_go_past_array_end (0 ms)
[----------] 9 tests from TestDataConverters (0 ms total)

[----------] Global test environment tear-down
[==========] 23 tests from 2 test suites ran. (0 ms total)
[  PASSED  ] 22 tests.
[  FAILED  ] 1 test, listed below:
[  FAILED  ] TestWavHeader.sample_rate_can_be_set

 1 FAILED TEST
```
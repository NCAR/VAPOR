# Render Regression Tests

** **For internal use**

### Components
- **run_config.py** Primary tool to run tests. It runs the entire suite of tests as configured in config.yaml. Run with `-h` for usage help.
- **run_test.py** Runs a single test render. Not intended to be used directly, it is used by the `run_config.py` script.
- **config.yaml** Contains the configuration for the list of tests to run. Split into 2 secions denoted by `--- # Section`. The first specifies global settings that apply to all tests. The second is a list of tests.

### Dependencies

The test requires vapor's python library to be installed as well as the pyyaml library.

### Output

The test will generate an output directory as specified in the config.yaml file. In the output directory, there will be one image file per test titled `<test name>-<renderer>[-<variables> (if multiple)].png`. In the output directory there will be a `meta` folder containing for each test a log file with the config used to generate the test and stdout/stderr from the test, and the session file that reproduces the test named `meta/<image>.log` and `meta/<image>.vs3` respectively.


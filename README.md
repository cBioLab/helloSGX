# helloSGX - Simple SGX Source Code

This program executes enclave initialization, ECALL, and OCALL. 

## Install
If your environment supports hardware mode, just execute following to build:

```
make
```

If you only can run simulation mode, execute following command to build:

```
make SGX_MODE=SIM
```

## Run program
Run the built program by following command:

```
./app
```

If the program is successfully executed, output like following will be displayed.

```
Execute ECALL.

Output from OCALL:
Hello Enclave.
=============================================================================
SGX_SUCCESS
Exited SGX function successfully.
=============================================================================

Returned integer from ECALL is: 31337

Whole operations have been executed correctly.
```

## LICENSE
MIT license is adopted for this project.
In addition to that, "Makefile" also follows Intel's certain license.
For detail of Intel's OSS license see [here](https://github.com/intel/linux-sgx/blob/master/License.txt).

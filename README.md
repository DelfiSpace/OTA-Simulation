# OTA-test
This is a C-program that emulates the flash memory of a microcontroller, so that a OTA(Over-the-air) software update can be implemented. 

### Compilation
Compile with:
```
make test=(USED_TEST)
```
Where (USED_TEST) is the name of a c-file in the test folder.

Compilation only works on Linux right now, because of the usage of the openssl library.

### Structure
The structure of this repository is as follows:
* root: This will contain the main code of the implementation.
* 'slots' folder: This folder contains files that emulate the slots of the flash memory
* 'tests' folder: This folder contains the test files to test the main code.

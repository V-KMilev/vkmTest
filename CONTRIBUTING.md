# Contributing to vkmTest

Thank you for wanting to help. This page is for changing vkmTest itself; a bug or a question goes in an [issue](https://github.com/V-KMilev/vkmTest/issues).

## Before you write code

Open an issue for anything bigger than a fix, so the shape is agreed before the work is done. Read the [README](README.md) first: its conventions are promises every suite relies on. A test's name, its exit codes and its folder are what CTest and the suites read, and a change that moves one says so and why.

## Build and test

vkmTest builds alone:

```sh
git clone https://github.com/V-KMilev/vkmTest
cd vkmTest
cmake -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

It is developed inside vkmEngine, which pins it at `modules/vkmTest` and builds every library's suite on it: a change is tested there too, with `ctest -L vkm_test`, every library's suite and the engine's own. The engine's [CONTRIBUTING.md](https://github.com/V-KMilev/vkmEngine/blob/master/CONTRIBUTING.md#build-and-test) says how to build it.

## What a pull request needs

A clean build with no warnings in GCC and Clang, a passing suite, a test you saw fail without your fix, and the README updated in the same commit. What the harness does with a failing test is tested by running one from `tests/fixture/`, never by weakening a check. A commit message is lowercase `type: summary`, then a few lines on what the change does as a whole and why. One subject per pull request, in as few commits as tell it.

## The license

vkmTest is source-available under the [vkmEngine License](LICENSE), not an open-source license. By submitting a contribution you grant the licensor the license its Contributions section describes, and confirm the work is yours to give. Changed code may be shared only as a contribution here or in a fork within this repository's GitHub fork network.

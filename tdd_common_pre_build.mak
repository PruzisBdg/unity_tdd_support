# --------------------------------------------------------------------------
#
# Defines common to the makefiles for all TDD unit tests in all projects
#
# ---------------------------------------------------------------------------

# Default TDD compiler: Make sure any libraries you link to are from this/compatible gcc.
TDD_C_COMPILER ?= gcc

# For the old PRUZISE6500 laptop. If returning to that then set TDD_C_COMPILER
#TDD_C_COMPILER ?= mingw32-gcc

# Default location for all Utility software and harnesses are here.
# But, really SPJ_SWR_LOC needs to be defined on any Host system.
SPJ_SWR_LOC ?= c:/spj6/swr

ifeq ($(OS),Windows_NT)
	TARGET_EXTENSION = .exe
    # Local box must have Unity framework here.
	UNITYDIR = c:/Unity/src/
	MAKE_TEST_RUNNER = ruby c:/unity/auto/generate_test_runner.rb
else
    # Linux Host
	TARGET_EXTENSION = .out
	UNITYDIR = /sandbox/Unity/
	MAKE_TEST_RUNNER = ruby c:/unity/auto/generate_test_runner.rb
endif

# Additional TDD support (other than Unity) is with all the Utility software.
TDD_SUPPORT_DIR = $(SPJ_SWR_LOC)/unity_tdd

# Final target is always .exe / .out. 'TARGET_BASE' is always defined for a harness
TARGET = $(TARGET_BASE)$(TARGET_EXTENSION)
OUT_FILE=-o $(TARGET)

# C-files are always this.
CEXT = .c

# Unit tests are relative to the base name.
DUT_SRC = $(TARGET_BASE)$(CEXT)\

# Unit tests are prefixed 'test-'
HARNESS_TESTS_SRC = test-$(DUT_SRC)

# Test runner main() is prefixed 'run-test-'
HARNESS_MAIN_SRC = run-$(HARNESS_TESTS_SRC)

# Utility libraries, if needed. Library paths are absolute because different harnesses are different levels down.
ARITH_LIB_NAME = arith_x86_gcc
ARITH_LIB_PATH = $(SPJ_SWR_LOC)/arith/arith_x86_gcc/bin/Debug
UTIL_LIB_NAME = util_x86_gcc
UTIL_LIB_PATH = $(SPJ_SWR_LOC)/util/codeblocks_gcc/bin/Debug
TINY2_LIB_NAME = tiny2_x86_gcc
TINY2_LIB_PATH = $(SPJ_SWR_LOC)/tiny2/codeblocks_gcc/bin/Debug

LIBS = -L $(ARITH_LIB_PATH) -L $(UTIL_LIB_PATH) -L $(TINY2_LIB_PATH) -l$(UTIL_LIB_NAME) -l$(ARITH_LIB_NAME) -l$(TINY2_LIB_NAME)

# All units are code coverage enabled, no optimisation, link math lib...
# (which is used sometimes, and must be explicitly included in some MinGW distributions)
CFLAGS = --coverage -O0 -lm

# For Linux??
SYMBOLS=-DTEST -DUNITY_TDD
BUILDDIR=../build

# All harnesses can see TDD C support code, Unity harness, the public interface for 'util'
# and the private dir for sharing among 'util'.
INC_DIRS = -I$(TDD_SUPPORT_DIR) -I../../private -I../../public -I../../GenericTypes -I$(UNITYDIR)
SRC_FILES = $(TDD_SUPPORT_DIR)/tdd_common.c

# ------------------------------ eof --------------------------------------

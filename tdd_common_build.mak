# --------------------------------------------------------------------------
#
# All TDD Unit Tests: Clean, Build and show coverage in Browser.
#
# ---------------------------------------------------------------------------

ifeq ($(OS),Windows_NT)
	CLEANUP = echo del /F /Q ..\build\* && echo del /F /Q $(TARGET) && echo del /F /Q *.html, *.gcno, *.gcda
else
	CLEANUP = rm -f build/*.o ; rm -f $(TARGET) ; mkdir -p build
endif

all: clean default

# Unless you 'set' otherwise; the coverage page will open in the default browser.
BROWSER ?= ""

# Builds test harness with coverage enabled, runs that harness, makes and shows coverage HTML.
default:
	$(MAKE_TEST_RUNNER) test-$(TARGET_BASE)$(CEXT)
	$(TDD_C_COMPILER) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) $(OUT_FILE)
	./$(TARGET)
	gcov $(TARGET_BASE)$(CEXT)
	gcovr -r . --html --html-details -o gcovr.html
	start $(BROWSER) "gcovr.html"

clean:
	$(CLEANUP)

addbuild:
	mkdir build

delbuild:
	rmdir build

# ------------------------------ eof -------------------------------------

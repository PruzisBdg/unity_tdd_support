# --------------------------------------------------------------------------
#
# All TDD Unit Tests: Clean, Build and show coverage in Browser.
#
# ---------------------------------------------------------------------------

ifeq ($(OS),Windows_NT)
	CLEANUP = echo del /F /Q ..\build\* && echo del /F /Q $(TARGET)
else
	CLEANUP = rm -f build/*.o ; rm -f $(TARGET) ; mkdir -p build
endif

all: clean default

# Unless you 'set' otherwise; the coverage page will open in the default browser.
BROWSER ?= ""

default:
	$(MAKE_TEST_RUNNER) test-$(TARGET_BASE)$(CEXT)
	$(TDD_C_COMPILER) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) $(OUT_FILE)
	./$(TARGET)
	gcov $(TARGET_BASE)$(CEXT)
	gcovr -r . --html --html-details -o gcovr.html
	# start "" "C:\Program Files (x86)\Mozilla Firefox\Firefox.exe" "-new-tab" "gcovr.html"
	start $(BROWSER) "gcovr.html"

clean:
	$(CLEANUP)

addbuild:
	mkdir build

delbuild:
	rmdir build

# ------------------------------ eof -------------------------------------

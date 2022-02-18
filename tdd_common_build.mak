# --------------------------------------------------------------------------
#
# All TDD Unit Tests: Clean, Build and show coverage in Browser.
#
# ---------------------------------------------------------------------------

ifeq ($(OS),Windows_NT)
	#PRECLEAN = echo del /F /Q ..\build\* && echo del /F /Q $(TARGET) && echo del /F /Q *.html, *.gcno, *.gcda
	
	# Despite being launched inside a 'cmd', make surely finds and open a MSys bash and then
	# tries to run 'del' from inside that. So spawn a 'cmd' to be sure del works. Doublequote '//c'
	# to get '/c' to cmd.
	PRECLEAN = cmd //c del $(TARGET), *.gcno, *.gcda, *.html, *.css
	POSTCLEAN = cmd //c del $(HARNESS_MAIN_SRC), $(TARGET), *.gcno, *.gcda
else
	PRECLEAN = rm -f build/*.o ; rm -f $(TARGET) ; mkdir -p build
endif

all: preclean default postclean

# Unless you 'set' otherwise; the coverage page will open in the default browser.
BROWSER ?= ""

# Builds test harness with coverage enabled, runs that harness, makes and shows coverage HTML.
default:
	$(MAKE_TEST_RUNNER) $(HARNESS_TESTS_SRC) $(HARNESS_MAIN_SRC)
	$(TDD_C_COMPILER) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) $(OUT_FILE)
	./$(TARGET)
	gcov $(TARGET_BASE)$(CEXT)
	gcovr -r . --html --html-details -o gcovr.html
	start $(BROWSER) "gcovr.html"

preclean:
	$(PRECLEAN)
	
postclean:
	$(POSTCLEAN)

addbuild:
	mkdir build

delbuild:
	rmdir build

# ------------------------------ eof -------------------------------------

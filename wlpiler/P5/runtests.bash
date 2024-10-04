#!/bin/bash
# runtests.bash - by Sylvie Davies for CS241 Winter 2020 
# (Updated a little for CS241 Fall 2023)
# Compares your output for a given sequence of test cases to the output of 
# the reference compiler wlp4c. 
# Usage: ./runtests.bash test1.wlp4 test2.wlp4 ...
# You can run as many tests as you like. The tests should be actual
# WLP4 programs ("Standard Format"). Each test "test.wlp4" should have a
# similarly named input file "test.in" in the same directory as the test.
# After running a test, symbolic links will be created in the current
# directory for easy access to the results. See the documentation of the 
# end() function for more details.

# The command used to run your compiler.
# Change this depending on what input format your compiler expects,
# whether you're using C++ or Racket, whether you want to run your
# C++ program with Valgrind, etc. See the PDF manual for details.
MY_COMPILER="wlp4scan | wlp4parse | wlp4type | ./a.out"

# The directory where test results are stored. 
TEST_DIR="runtests_dir"

# Time limit for tests in seconds. Only applies to the MIPS
# part of the tests (no time limit on how long your compiler takes).
TIMEOUT=5

# These "markers" are used to differentiate expected output (wlp4c) from
# the user's actual compiler output. For example, the user's output would
# be in a file called test.wlp4.u.out while the expected output would be
# in test.wlp4.x.out.
EXPECTED_MARKER="x"
USER_MARKER="u"

# Names of the print and alloc libraries
PRINT="print.merl"
ALLOC="alloc.merl"

# setup() ensures that:
# - CS241 tools are accessible
# - The test directory exists
# - The test runner knows whether print and alloc libraries are available
# - The runmips helper script exists and is executable
function setup() {
  source /u/cs241/setup
  if [ ! -d "$TEST_DIR" ]; then
    mkdir "$TEST_DIR"
  fi
  if [ -f "$PRINT" ]; then
    cp "$PRINT" "$TEST_DIR"
  else
    # If the user is missing print.merl, we indicate this by setting
    # the PRINT variable to an empty string.
    PRINT=""
  fi
  if [ -f "$ALLOC" ]; then
    cp "$ALLOC" "$TEST_DIR"
  else
    # If the user is missing alloc.merl, we indicate this by setting
    # the ALLOC variable to an empty string.
    ALLOC=""
  fi
  # Output the helper script used for running MIPS files.
  # This is a separate script instead of a function because of the way
  # the timeout command works - it doesn't seem to output to files when
  # using output redirection so instead we use a script that creates
  # the output files and call the script with timeout.

cat <<'runMipsScript' > "$TEST_DIR/runmips"
#!/bin/bash
# runmips() runs the MIPS program corresponding to the given program name
# and file marker, returning the exit code and producing output files.
# Also checks if the input file is valid.
function runmips() {
  local MIPS="$1"
  local PROGRAM="$2"
  local MARKER="$3"
  local INPUT="${PROGRAM%.*}.in"
  "$MIPS" "$PROGRAM.$MARKER.mips" < "$INPUT" > "$PROGRAM.$MARKER.out" 2> "$PROGRAM.$MARKER.err"
  return $?
}

runmips "$1" "$2" "$3"
exit $?
runMipsScript

  chmod u+x "$TEST_DIR/runmips"
}

# clean() deletes test files associated with the given program name,
# to ensure we don't reuse old data when re-running tests.
function clean() {
  local PROGRAM="$1"
  local FILES=$(ls $PROGRAM.*.* 2> /dev/null)
  if [ ! -z "$FILES" ]; then
    for FILE in $(echo $FILES); do
      rm "$FILE"
    done
  fi
}

# detect_mips() returns either "mips.twoints" or "mips.array" depending on 
# which one should be used for the given test case.
function detect_mips() {
  local PROGRAM="$1"
  # This is a bit complicated. First we scan the program:
  wlp4scan < "$PROGRAM" > "$PROGRAM.scanned_temp"
  # Now we look for the line number where WAIN appears and extract it with
  # grep and cut. Then we add 3. Why 3? Because if the program requires
  # mips.array, the scanner output will look like:
  # Line #  Output
  # ...     ...
  # n       WAIN wain
  # n+1     LPAREN (
  # n+2     INT int
  # n+3     STAR *
  # ...     ...
  # That's how we identify whether the first parameter is int or int*.
  # We look +3 lines ahead from where WAIN appears.
  local LINE=$(( $(grep -n -m 1 "WAIN" "$PROGRAM.scanned_temp" | cut -d: -f1) + 3 ))
  # More Bash nonsense: now that we know the line number, extract precisely 
  # that line using sed, then check for "STAR *" using grep.
  if sed -n "${LINE}p;$(( LINE + 1 ))q" "$PROGRAM.scanned_temp" | grep "STAR *" > /dev/null; then
    echo "mips.array"  
  else
    # If we don't find a STAR then the program must have two int parameters.
    echo "mips.twoints"
  fi
  rm "$PROGRAM.scanned_temp"
}

# validate() checks if the input file for the program is correct, e.g., if the
# input for mips.twoints consists of two integers.
function validate() {
  local PROGRAM="$1"
  local MIPS="$2"
  local INPUT="${PROGRAM%.*}.in"
  # Strip all blank lines or space-only lines
  cat "$INPUT" | grep "\S" > "$INPUT.stripped_temp"
  if [ "$MIPS" == "mips.array" ]; then
    # Get the number of lines in the file.
    local LINECOUNT="$(cat "$INPUT.stripped_temp" | wc -l)"
    # Get the number of lines the file should have by extracting the first
    # number in the file and adding one for the first line.
    local EXPECTED="$(( $(head -n1 "$INPUT.stripped_temp") + 1 ))"
    rm "$INPUT.stripped_temp"
    if [ "$LINECOUNT" -lt "$EXPECTED" ]; then
      echo "ERROR: $MIPS input array has too few numbers."
      return 1;
    fi
    if [ "$LINECOUNT" -gt "$EXPECTED" ]; then
      echo "WARNING: $MIPS input array has too many numbers."
      return 0;
    fi
  else
    local LINECOUNT="$(cat "$INPUT.stripped_temp" | wc -l)"
    rm "$INPUT.stripped_temp"
    # For mips.twoints we just check for two lines.
    if [ "$LINECOUNT" -lt "2" ]; then
      echo "ERROR: $MIPS input has too few numbers."
      return 1;
    fi
    if [ "$LINECOUNT" -gt "2" ]; then
      echo "WARNING: $MIPS input has too many numbers."
      return 0;
    fi
  fi
  return 0;
}

# extract_reg3() extracts the value of register 3 from the standard error 
# output of the MIPS simulator (twoints or array).
function extract_reg3() {
  # grep finds the line with "$03", the cuts extract the value.
  echo $(grep "\$03" $1 | cut -d'=' -f4 | cut -d'$' -f1)
}

# makelink() creates a symbolic link using the given program name,
# marker and extension. Helper function for end().
function makelink() {
  local PROGRAM="$1"
  local MARKER="$2"
  local EXTENSION="$3"
  if [ -f "$(basename $PROGRAM).$MARKER.$EXTENSION" ]; then 
    unlink "../.${MARKER}${EXTENSION}" &> /dev/null
    ln -s "$TEST_DIR/$(basename $PROGRAM).$MARKER.$EXTENSION" "../.${MARKER}${EXTENSION}"
  fi
}

# end() runs after a test finishes to create symbolic links to important
# output files, and to switch back to the original directory.
# The symbolic links let you quickly view the results of the most recent
# test. For example, if EXPECTED_MARKER and USER_MARKER have their default
# values of "x" and "u", then ".xout" gives you the expected output, while
# ".uout" gives your output. The symbolic links created are:
# .xout  - MIPS simulator stdout for reference compiler
# .xerr  - MIPS simulator stderr (register contents) for reference compiler
# .xreg3 - MIPS simulator $3 value for user's compiler
# .uout  - MIPS simulator stdout for user's compiler
# .uerr  - MIPS simulator stderr (register contents) for user's compiler
# .ureg3 - MIPS simulator $3 value for user's compiler
# .uasm  - user's compiler assembly output
# .in    - standard input file for the test
# These apply to the most recently run test. If you use this script to run
# a suite of tests, these symbolic links will point to the results of the
# last test in the suit.
function end() {
  local PROGRAM="$1" 
  makelink "$PROGRAM" "$EXPECTED_MARKER" "out"
  makelink "$PROGRAM" "$EXPECTED_MARKER" "err"
  makelink "$PROGRAM" "$EXPECTED_MARKER" "reg3"
  makelink "$PROGRAM" "$USER_MARKER" "out"
  makelink "$PROGRAM" "$USER_MARKER" "err"
  makelink "$PROGRAM" "$USER_MARKER" "reg3"
  makelink "$PROGRAM" "$USER_MARKER" "asm"
  # Linking the test input is a bit different so we don't use makelink.
  if [ -f "../${PROGRAM%.*}.in" ]; then 
    unlink "../.in" &> /dev/null
    ln -s "${PROGRAM%.*}.in" "../.in"
  fi
  cd ..
}

# runtest() runs an individual test.
# Test input files are assumed to be named "testname.in" where the test input
# file is called "testname.wlp4". Input files should be in the same directory
# as the corresponding test. If no input file exists, you will be prompted
# to create one.
function runtest() {
  local PROGRAM="$1"
  # Copy the program to the test directory
  if ! cp "$PROGRAM" "$TEST_DIR"; then
    echo "ERROR: Test program does not exist?"
    return 1;
  fi
  local MIPS="$(detect_mips $PROGRAM)"
  # Check if test input exists - if not prompt the user - then copy it
  # to the test directory. ${PROGRAM%.*} gets the filename without extension.
  if [ ! -f "${PROGRAM%.*}.in" ]; then
    echo "No test input for $PROGRAM (uses $MIPS)."
    echo "Enter one below and press Ctrl+D when done."
    cat > "${PROGRAM%.*}.in"
  fi
  cp "${PROGRAM%.*}.in" "$TEST_DIR"
  cd "$TEST_DIR"
  # Time to test the program...
  echo "Testing $PROGRAM with input file ${PROGRAM%.*}.in..."
  # Get the basename to avoid issues if the user stores their tests
  # in a directory. For example if PROGRAM is tests/mytest.wlp4 then
  # we actually just want the "mytest.wlp4" part.
  PROGRAM=$(basename $PROGRAM)
  clean "$PROGRAM"
  if ! validate "$PROGRAM" "$MIPS"; then 
    end "$1"
    return 1;
  fi
  # Check if the program compiles.
  if wlp4c < "$PROGRAM" > "$PROGRAM.$EXPECTED_MARKER.mips"; then
    # If so, compute the expected output.
    # We run the MIPS simulator with a timeout.
    # We also use a special "runmips" script instead of calling the MIPS
    # simulator more directly - see the comments in setup().
    if ! timeout $TIMEOUT ./runmips "$MIPS" "$PROGRAM" "$EXPECTED_MARKER"; then 
      echo "ERROR: Test exceeded time limit of $TIMEOUT seconds (reference compiler)."
      echo "Your test program might have an infinite loop or infinite recursion."
      # We pass the full path to the end function instead of the basename.
      end "$1"
      return 1;
    fi
    # Check if the MIPS simulator crashed.
    local ERR_EXPECTED=$(grep "MIPS emulator internal error" "$PROGRAM.$EXPECTED_MARKER.err")
    if [ ! -z "$ERR_EXPECTED" ]; then
      echo "ERROR: MIPS emulator crashed while running reference compiler's output."
      echo "Either your program does something bad like dereferencing NULL,"
      echo "or (less likely) there is a bug in the reference compiler."
      tail -n10 "$PROGRAM.$EXPECTED_MARKER.err"
      end "$1"
      return 1;
    fi
    extract_reg3 "$PROGRAM.$EXPECTED_MARKER.err" > "$PROGRAM.$EXPECTED_MARKER.reg3"
    # Now compute the user's output.
    # First run the scanner, parser and compiler.
    # We hop out of the test directory to make sure the command works properly.
    # We cat the full pathname instead of the basename.
    cd ..
    bash -c "cat "$1" | $MY_COMPILER" > "$TEST_DIR/$PROGRAM.$USER_MARKER.asm"
    cd "$TEST_DIR"
    # Now run the assembler...
    cs241.linkasm < "$PROGRAM.$USER_MARKER.asm" > "$PROGRAM.$USER_MARKER.merl"
    # Next is the linker.
    # Don't run the linker if BOTH print.merl and alloc.merl are missing.
    if [ -z "${PRINT}${ALLOC}" ]; then
      cp "$PROGRAM.$USER_MARKER.merl" "$PROGRAM.$USER_MARKER.mips"
    else 
      cs241.linker "$PROGRAM.$USER_MARKER.merl" $PRINT $ALLOC > "$PROGRAM.$USER_MARKER.mips"
    fi
    # Finally, run the actual MIPS program.
    if ! timeout $TIMEOUT ./runmips "$MIPS" "$PROGRAM" "$USER_MARKER"; then 
      echo "Test exceeded time limit of $TIMEOUT seconds (user's compiler)."
      end "$1"
      return 0;
    fi
    local ERR_USER=$(grep "MIPS emulator internal error" "$PROGRAM.$USER_MARKER.err")
    if [ ! -z "$ERR_USER" ]; then
      echo "MIPS emulator crashed while running user's compiler output."
      tail -n10 "$PROGRAM.$USER_MARKER.err"
      end "$1"
      return 0;
    fi
    extract_reg3 "$PROGRAM.$USER_MARKER.err" > "$PROGRAM.$USER_MARKER.reg3"
    # Compare standard output.
    local STDOUT=$(diff "$PROGRAM.$EXPECTED_MARKER.out" "$PROGRAM.$USER_MARKER.out")
    if [ -z "$STDOUT" ]; then
     echo "Standard output is correct!"
    else
     echo "Standard output is incorrect."
     echo "Diff below (expected output left, user output right):"
     echo "diff -y $TEST_DIR/$PROGRAM.$EXPECTED_MARKER.out $TEST_DIR/$PROGRAM.$USER_MARKER.out"
     # Use -y for a side-by-side diff
     diff -y "$PROGRAM.$EXPECTED_MARKER.out" "$PROGRAM.$USER_MARKER.out"
    fi
    # Compare return value in register 3.
    local REG=$(diff "$PROGRAM.$EXPECTED_MARKER.reg3" "$PROGRAM.$USER_MARKER.reg3")
    if [ -z "$REG" ]; then
     echo "Return value in \$3 is correct!"
    else
     echo "Return value in \$3 is incorrect."
     echo "Diff below (expected output top, user output bottom):"
     echo "diff $TEST_DIR/$PROGRAM.$EXPECTED_MARKER.reg3 $TEST_DIR/$PROGRAM.$USER_MARKER.reg3"
     echo "$REG"
    fi
  else
    # If we ended up down here, the test program doesn't compile.
    echo "ERROR: Test program $PROGRAM is invalid."
    end "$1"
    return 1;
  fi
  end "$1"
  return 0;
}

# Main program: do setup then run runtest on each test in the arguments.
setup 
for t in $*; do
  runtest "$t"
  echo
done
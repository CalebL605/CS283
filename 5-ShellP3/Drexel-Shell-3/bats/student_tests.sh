#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Built-in exit: prints \"exit...\" and returns code 0" {
    run ./dsh <<EOF
exit
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh3>exiting...cmdloopreturned0"

    echo "Captured output: $output"
    echo "Stripped output: $stripped_output"
    echo "Expected output: $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Built-in cd: change directory when given one argument" {
    current_dir=$(pwd)
    cd /tmp
    mkdir -p dsh_test_cd

    run "${current_dir}/dsh" <<EOF
cd dsh_test_cd
pwd
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="/tmp/dsh_test_cddsh3>dsh3>dsh3>cmdloopreturned0"

    echo "Captured output: $output"
    echo "Stripped output: $stripped_output"
    echo "Expected output: $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Built-in cd: no argument should not change directory" {
    current_dir=$(pwd)

    run "${current_dir}/dsh" <<EOF
cd
pwd
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="${current_dir}dsh3>dsh3>dsh3>cmdloopreturned0"

    echo "Captured output: $output"
    echo "Stripped output: $stripped_output"
    echo "Expected output: $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Built-in cd: wrong argument should return error and not change directory" {
    current_dir=$(pwd)

    run "${current_dir}/dsh" <<EOF
cd no_such_directory
pwd
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="${current_dir}dsh3>error:couldnotchangedirectorytono_such_directorydsh3>dsh3>cmdloopreturned0"

    echo "Captured output: $output"
    echo "Stripped output: $stripped_output"
    echo "Expected output: $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Built-in cd: too many arguments should return error and not change directory" {
    current_dir=$(pwd)
    cd /tmp
    mkdir -p dsh_test_cd

    run "${current_dir}/dsh" <<EOF
cd dsh_test_cd arg2
pwd
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="/tmpdsh3>error:toomanyargumentsforcddsh3>dsh3>cmdloopreturned0"

    echo "Captured output: $output"
    echo "Stripped output: $stripped_output"
    echo "Expected output: $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Built-in dragon: displays ASCII Drexel dragon art" {
    run ./dsh <<EOF
dragon
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    
    [[ "$stripped_output" == "dsh3>@%%%%"* ]]
    [[ "$stripped_output" == *"%%%%%%%@dsh3>cmdloopreturned0" ]]
    [ "$status" -eq 0 ]
}

@test "Echo without quotes: spaces gets handled correctly" {
    run ./dsh <<EOF
echo      multiple     spaces
EOF
    # Ensure the output contains both "multiple" and "spaces" with only a single space between them.
    [[ "$output" == *"multiple spaces"* ]]
    [ "$status" -eq 0 ]
}

@test "Echo with quoted spaces: preserves internal spaces" {
    run ./dsh <<EOF
echo "   hello    world   "
exit
EOF

    # Remove newline and tab characters but preserve spaces.
    processed_output=$(echo "$output" | tr -d '\n\r\t')
    expected_fragment="   hello    world   "

    echo "Captured output: $processed_output"

    [[ "$processed_output" == *"$expected_fragment"* ]]
    [ "$status" -eq 0 ]
}

@test "External command: uname returns expected substring Linux" {
    run ./dsh <<EOF
uname
EOF

    # Look for an expected output "Linux".
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="Linuxdsh3>dsh3>cmdloopreturned0"

    echo "Captured output: $output"
    echo "Stripped output: $stripped_output"
    echo "Expected output: $expected_output"

    [[ "$stripped_output" == "$expected_output" ]]
    [ "$status" -eq 0 ]
}

@test "External command: pwd displays current directory" {
    current_dir=$(pwd)
    run ./dsh <<EOF
pwd
EOF
    # Ensure the output contains the current directory.
    [[ "$output" == *"$current_dir"* ]]
    [ "$status" -eq 0 ]
}

@test "External command with arguments: ls -l produces output" {
  run ./dsh <<EOF
ls -l
EOF
  # Check that ls -l returns some output (non-empty).
  [ -n "$output" ]
  [ "$status" -eq 0 ]
}

@test "Invalid external command: returns ENOENT error message" {
    run ./dsh <<EOF
nonexistentcommand
EOF
    # Check that the error message gets returned
    [[ "$output" == *"Error: Command not found in PATH"* ]]
    [ "$status" -eq 0 ]
}

@test "Invalid external command: EACCES error message" {
    # Create a temporary file that is not executable.
    tmpfile=$(mktemp /tmp/no_exec.XXXX)
    echo "#!/bin/sh" > "$tmpfile"
    echo "echo 'This should not run'" >> "$tmpfile"
    chmod -x "$tmpfile"  # Remove execute permissions

    run ./dsh <<EOF
$tmpfile
EOF

    # Check that the EACCES error message appears.
    [[ "$output" == *"Error: Permission denied"* ]]
    [ "$status" -eq 0 ]
    
    rm -f "$tmpfile"
}

@test "Invalid external command: Default error message for other error codes" {
    run ./dsh <<EOF
false
EOF

    # Check that the default error message appears
    [[ "$output" == *"Command execution failed with code 1"* ]]
    [ "$status" -eq 0 ]
}

@test "Built-in rc: displays code 2 for external command with ENOENT error" {
    run ./dsh <<EOF
not_exists
rc
EOF
  
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh3>dsh3>Error:CommandnotfoundinPATHdsh3>2dsh3>cmdloopreturned0"

    echo "Captured output: $output"
    echo "Stripped output: $stripped_output"
    echo "Expected output: $expected_output"

    # Check that the output is the expected error message
    [[ "$stripped_output" == "$expected_output" ]]
    [ "$status" -eq 0 ]
}

@test "Built-in rc: displays 0 for a successful command" {
    run ./dsh <<EOF
echo "success"
rc
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="successdsh3>dsh3>0dsh3>cmdloopreturned0"

    echo "Captured output: $output"
    echo "Stripped output: $stripped_output"
    echo "Expected output: $expected_output"

    # Check that the output is the expected error message
    [[ "$stripped_output" == "$expected_output" ]]
    [ "$status" -eq 0 ]
}

@test "Pipe limit: More than 8 piped commands returns an error message" {
    cmd="echo one | echo two | echo three | echo four | echo five | echo six | echo seven | echo eight | echo nine"
    run ./dsh <<EOF
$cmd
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh3>error:pipinglimitedto8commandsdsh3>cmdloopreturned-2"

    echo "Captured output: $output"
    echo "Stripped output: $stripped_output"
    echo "Expected output: $expected_output"

    # Check that the output is the expected error message
    [[ "$stripped_output" = "$expected_output" ]]
    [ "$status" -eq 0 ]
}

@test "Pipe: echo piped to tr converts lowercase to uppercase" {
    run ./dsh <<EOF
echo hello world | tr a-z A-Z
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="HELLOWORLDdsh3>dsh3>cmdloopreturned0"

    echo "Captured output: $output"
    echo "Stripped output: $stripped_output"
    echo "Expected output: $expected_output"

    # Check that the output is the uppercase version of the input
    [[ "$stripped_output" == "$expected_output" ]]
    [ "$status" -eq 0 ]
}

@test "Pipe: multiple commands in pipeline (echo | tr | rev)" {
    run ./dsh <<EOF
echo "hello world" | tr a-z A-Z | rev
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="DLROWOLLEHdsh3>dsh3>cmdloopreturned0"

    echo "Captured output: $output"
    echo "Stripped output: $stripped_output"
    echo "Expected output: $expected_output"
    
    # Check that the output is the reverse of the uppercase text
    [[ "$stripped_output" == "$expected_output" ]]
    [ "$status" -eq 0 ]
}

@test "Redirection: output redirection using > (echo > file then cat file)" {
    tmpfile=$(mktemp /tmp/redir_test.XXXX)
    run ./dsh <<EOF
echo "testoutput" > $tmpfile
cat $tmpfile
EOF

    # Expect file contents to match
    [[ "$output" == *"testoutput"* ]]
    [ "$status" -eq 0 ]
    rm -f "$tmpfile"
}

@test "Redirection: append redirection using >> (first echo >, then >>, then cat)" {
    tmpfile=$(mktemp /tmp/redir_test.XXXX)
    run ./dsh <<EOF
echo "line1" > $tmpfile
echo "line2" >> $tmpfile
cat $tmpfile
EOF
    # Expect file to contain both lines.
    [[ "$output" == *"line1"* ]]
    [[ "$output" == *"line2"* ]]
    [ "$status" -eq 0 ]
    rm -f "$tmpfile"
}

@test "Pipe and Redirection: external command pipeline with output redirection" {
    tmpfile=$(mktemp /tmp/redir_pipe.XXXX)
    run ./dsh <<EOF
echo "pipe test" | tr a-z A-Z > $tmpfile
cat $tmpfile
EOF

    # Expect file contents to be uppercase "PIPE TEST"
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    [[ "$stripped_output" == *"PIPETEST"* ]]
    [ "$status" -eq 0 ]
    rm -f "$tmpfile"
}

@test "Pipe and Redirection: built-in dragon with output redirection" {
    tmpfile=$(mktemp /tmp/redir_dragon.XXXX)
    run ./dsh <<EOF
dragon > $tmpfile
exit
EOF

    # Since dragon prints ASCII art, check that the redirected file has content with '%' characters.
    file_content=$(cat $tmpfile | tr -d '[:space:]')
    [[ "$file_content" == *@%* ]]
    [ "$status" -eq 0 ]
    rm -f "$tmpfile"
}

@test "Pipe and Redirection: built-in rc with output redirection" {
    tmpfile=$(mktemp /tmp/redir_rc.XXXX)
    run ./dsh <<EOF
echo "success"
rc > $tmpfile
EOF

    # Expect file to contain the rc code (likely 0) as shown in output.
    file_content=$(cat $tmpfile | tr -d '[:space:]')
    [[ "$file_content" == *"0"* ]]
    [ "$status" -eq 0 ]
    rm -f "$tmpfile"
}

@test "Input Redirection: cat with input redirection" {
    tmpfile=$(mktemp /tmp/redir_in.XXXX)
    echo "inputredirection" > "$tmpfile"

    run ./dsh <<EOF
cat < $tmpfile
EOF

    # Expect output to match file content.
    [[ "$output" == *"inputredirection"* ]]
    [ "$status" -eq 0 ]
    rm -f "$tmpfile"
}

@test "Pipeline with Input and Output Redirection: cat with input, piped to tr, redirected output" {
    tmpfile_in=$(mktemp /tmp/redir_in.XXXX)
    tmpfile_out=$(mktemp /tmp/redir_out.XXXX)
    echo "mixedCaseText" > "$tmpfile_in"

    run ./dsh <<EOF
cat < $tmpfile_in | tr a-z A-Z > $tmpfile_out
cat $tmpfile_out
EOF

    # Expect output to be uppercase version of the text.
    [[ "$output" == *"MIXEDCASETEXT"* ]]
    [ "$status" -eq 0 ]
    rm -f "$tmpfile_in" "$tmpfile_out"
}

@test "Complex Pipeline: multiple commands with redirection" {
    tmpfile=$(mktemp /tmp/complex_pipe.XXXX)
    run ./dsh <<EOF
echo "hello world from shell" | tr a-z A-Z | rev > $tmpfile
cat $tmpfile
EOF

    # Check that the output is the complex result (reverse of uppercase).
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    [[ "$stripped_output" == *"LLEHSMORFDLROWOLLEH"* ]]
    [ "$status" -eq 0 ]
    rm -f "$tmpfile"
}

@test "Normal command redirection (no pipes): echo redirected to file and then cat file" {
    tmpfile=$(mktemp /tmp/redir_normal.XXXX)
    run ./dsh <<EOF
echo "normal redirection test" > $tmpfile
cat $tmpfile
EOF

    # Expect the output to include "normal redirection test"
    [[ "$output" == *"normal redirection test"* ]]
    [ "$status" -eq 0 ]
    rm -f "$tmpfile"
}
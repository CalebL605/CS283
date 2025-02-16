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

@test "Built-in exit: returns correct error code -7" {
    run ./dsh <<EOF
exit
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh2>cmdloopreturned-7"

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
    expected_output="/tmp/dsh_test_cddsh2>dsh2>dsh2>cmdloopreturned0"

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
    expected_output="${current_dir}dsh2>dsh2>dsh2>cmdloopreturned0"

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
    expected_output="${current_dir}dsh2>error:couldnotchangedirectorytono_such_directorydsh2>dsh2>cmdloopreturned0"

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
    expected_output="/tmpdsh2>error:toomanyargumentsforcddsh2>dsh2>cmdloopreturned0"

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
    
    [[ "$stripped_output" == "dsh2>@%%%%"* ]]
    [[ "$stripped_output" == *"%%%%%%%@dsh2>cmdloopreturned0" ]]
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

@test "External command: uname -a returns expected substring" {
    run ./dsh <<EOF
uname
exit
EOF
    # Look for an expected output "Linux".
    [[ "$output" == *"Linux"* ]]
    [ "$status" -eq 0 ]
}

@test "External command: pwd displays current directory" {
    current_dir=$(pwd)
    run ./dsh <<EOF
pwd
exit
EOF
    # Ensure the output contains the current directory.
    [[ "$output" == *"$current_dir"* ]]
    [ "$status" -eq 0 ]
}

@test "External command with arguments: ls -l produces output" {
  run ./dsh <<EOF
ls -l
exit
EOF
  # Check that ls -l returns some output (non-empty).
  [ -n "$output" ]
  [ "$status" -eq 0 ]
}

@test "Invalid external command: returns ENOENT error message" {
    run ./dsh <<EOF
nonexistentcommand
exit
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
exit
EOF

    # Check that the EACCES error message appears.
    [[ "$output" == *"Error: Permission denied"* ]]
    [ "$status" -eq 0 ]
    
    rm -f "$tmpfile"
}

@test "Invalid external command: Default error message for other error codes" {
    run ./dsh <<EOF
false
exit
EOF

  [[ "$output" == *"Command execution failed with code 1"* ]]
  [ "$status" -eq 0 ]
}

@test "Built-in rc: displays code 2 for external command with ENOENT error" {
    run ./dsh <<EOF
not_exists
rc
exit
EOF
  
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh2>dsh2>Error:CommandnotfoundinPATHdsh2>2dsh2>cmdloopreturned-7"

    echo "Captured output: $output"
    echo "Stripped output: $stripped_output"
    echo "Expected output: $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Built-in rc: displays 0 for a successful command" {
    run ./dsh <<EOF
echo "success"
rc
exit
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="successdsh2>dsh2>0dsh2>cmdloopreturned-7"

    echo "Captured output: $output"
    echo "Stripped output: $stripped_output"
    echo "Expected output: $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}
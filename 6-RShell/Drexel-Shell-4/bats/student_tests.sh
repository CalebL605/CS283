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
    expected_output="localmodedsh4>exiting...cmdloopreturned0"

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
    expected_output="/tmp/dsh_test_cdlocalmodedsh4>dsh4>dsh4>cmdloopreturned0"

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
    expected_output="${current_dir}localmodedsh4>dsh4>dsh4>cmdloopreturned0"

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
    expected_output="${current_dir}localmodedsh4>error:couldnotchangedirectorytono_such_directorydsh4>dsh4>cmdloopreturned0"

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
    expected_output="/tmplocalmodedsh4>error:toomanyargumentsforcddsh4>dsh4>cmdloopreturned0"

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
    
    [[ "$stripped_output" == "localmodedsh4>@%%%%"* ]]
    [[ "$stripped_output" == *"%%%%%%%@dsh4>cmdloopreturned0" ]]
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
    expected_output="Linuxlocalmodedsh4>dsh4>cmdloopreturned0"

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
    expected_output="localmodedsh4>localmodedsh4>Error:CommandnotfoundinPATHdsh4>2dsh4>cmdloopreturned0"

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
    expected_output="successlocalmodedsh4>dsh4>0dsh4>cmdloopreturned0"

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
    expected_output="localmodedsh4>error:pipinglimitedto8commandsdsh4>cmdloopreturned-2"

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
    expected_output="HELLOWORLDlocalmodedsh4>dsh4>cmdloopreturned0"

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
    expected_output="DLROWOLLEHlocalmodedsh4>dsh4>cmdloopreturned0"

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

@test "Usage: ./dsh -h prints help message" {
    run ./dsh -h 

    # Check that the usage message contains the expected options.
    [[ "$output" =~ "Usage: ./dsh [-c | -s] [-i IP] [-p PORT] [-x] [-h]" ]]
    [[ "$output" =~ "-c" ]]
    [[ "$output" =~ "-s" ]]
    [[ "$output" =~ "-i IP" ]]
    [[ "$output" =~ "-p PORT" ]]
    [[ "$output" =~ "-x" ]]
    [[ "$output" =~ "-h" ]]
    [ "$status" -eq 0 ]
}

@test "Remote Client: Server starts up on default port" {
    ./dsh -s &
    server_pid=$!
    sleep 1

    run ./dsh -c <<EOF
EOF

    kill $server_pid

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    echo "Stripped output: $stripped_output"

    [[ "$stripped_output" == "socketclientmode:addr:127.0.0.1:1234rsh>cmdloopreturned0" ]]
    [ "$status" -eq 0 ]
}

@test "Remote Client: Server starts up on custom port" {
    ./dsh -s -p 8080 &
    server_pid=$!
    sleep 1

    run ./dsh -c -p 8080 <<EOF
EOF

    kill $server_pid
    
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    echo "Stripped output: $stripped_output"

    [[ "$stripped_output" == "socketclientmode:addr:127.0.0.1:8080rsh>cmdloopreturned0" ]]
    [ "$status" -eq 0 ]
}

@test "Remote Client: exit stops client but not server" {
    ./dsh -s -p 6790 &
    server_pid=$!
    sleep 1

    # First client
    run ./dsh -c -p 6790 <<EOF
exit
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    echo "Stripped output: $stripped_output"

    [[ "$stripped_output" == "socketclientmode:addr:127.0.0.1:6790rsh>exiting...cmdloopreturned0" ]]
    [ "$status" -eq 0 ]

    # Second client
    run ./dsh -c -p 6790 <<EOF
echo "still working"
exit
EOF

    kill $server_pid

    [[ "$output" =~ "still working" ]]
    [ "$status" -eq 0 ]
}

@test "Remote Client: stop-server command" {
    ./dsh -s -p 6791 &
    server_pid=$!
    sleep 1

    run ./dsh -c -p 6791 <<EOF
stop-server
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    echo "Stripped output: $stripped_output"

    [[ "$stripped_output" == *"rsh>exiting...cmdloopreturned0" ]]
    [ "$status" -eq 0 ]

    # Try to connect again (should fail)
    run ./dsh -c -p 6791 <<EOF
echo "test"
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    echo "Stripped output: $stripped_output"

    [[ "$stripped_output" == *"cmdloopreturned-52" ]]
    [ "$status" -eq 0 ]
}

@test "Remote Client: Execute built-in cd command" {
    ./dsh -s -p 6792 &
    server_pid=$!
    sleep 1

    run ./dsh -c -p 6792 <<EOF
cd /tmp
pwd
exit
EOF

    kill $server_pid
    [[ "$output" =~ "/tmp" ]]
    [ "$status" -eq 0 ]
}

@test "Remote Client: Execute built-in dragon command" {
    ./dsh -s -p 6793 &
    server_pid=$!
    sleep 1

    run ./dsh -c -p 6793 <<EOF
dragon
exit
EOF

    kill $server_pid
    [[ "$output" =~ "@" ]]
    [[ "$output" =~ "%" ]]
    [ "$status" -eq 0 ]
}

@test "Remote Client: Execute built-in rc command" {
    ./dsh -s -p 6794 &
    server_pid=$!
    sleep 1

    run ./dsh -c -p 6794 <<EOF
echo "success"
rc
exit
EOF

    kill $server_pid
    [[ "$output" =~ "0" ]]
    [ "$status" -eq 0 ]
}

@test "Remote Client: Execute external command ls" {
    ./dsh -s -p 6795 &
    server_pid=$!
    sleep 1

    run ./dsh -c -p 6795 <<EOF
ls
exit
EOF

    kill $server_pid
    [ -n "$output" ]
    [ "$status" -eq 0 ]
}

@test "Remote Client: Execute external command pwd" {
    ./dsh -s -p 6796 &
    server_pid=$!
    sleep 1

    current_dir=$(pwd)
    run ./dsh -c -p 6796 <<EOF
pwd
exit
EOF

    kill $server_pid
    [[ "$output" =~ "$current_dir" ]]
    [ "$status" -eq 0 ]
}

@test "Remote Client: Execute external command uname" {
    ./dsh -s -p 6797 &
    server_pid=$!
    sleep 1

    run ./dsh -c -p 6797 <<EOF
uname
exit
EOF

    kill $server_pid
    [[ "$output" =~ "Linux" ]]
    [ "$status" -eq 0 ]
}

@test "Remote Client: Simple pipe test" {
    ./dsh -s -p 6798 &
    server_pid=$!
    sleep 1

    run ./dsh -c -p 6798 <<EOF
echo "hello world" | tr a-z A-Z
exit
EOF

    kill $server_pid
    [[ "$output" =~ "HELLO WORLD" ]]
    [ "$status" -eq 0 ]
}

@test "Remote Client: Complex pipe test" {
    ./dsh -s -p 6799 &
    server_pid=$!
    sleep 1

    run ./dsh -c -p 6799 <<EOF
echo "hello world" | tr a-z A-Z | rev
exit
EOF

    kill $server_pid
    [[ "$output" =~ "DLROW OLLEH" ]]
    [ "$status" -eq 0 ]
}

@test "Remote Client: Invalid command handling" {
    ./dsh -s -p 6800 &
    server_pid=$!
    sleep 1

    run ./dsh -c -p 6800 <<EOF
nonexistentcommand
exit
EOF

    kill $server_pid
    [[ "$output" =~ "Error: Command not found in PATH" ]]
    [ "$status" -eq 0 ]
}

@test "Remote Client: Connection refused test" {
    run ./dsh -c -p 6801 <<EOF
echo "test"
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    echo "Stripped output: $stripped_output"

    [[ "$stripped_output" == "socketclientmode:addr:127.0.0.1:6801cmdloopreturned-52" ]]
    [ "$status" -eq 0 ]
}

@test "Remote Client: Servers of the same port cannot co-exist" {
    ./dsh -s -p 6802 &
    server_pid=$!
    sleep 1

    run ./dsh -s -p 6802 <<EOF
EOF

    kill $server_pid
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    echo "Stripped output: $stripped_output"

    [[ "$stripped_output" == "socketservermode:addr:0.0.0.0:6802->Single-ThreadedModecmdloopreturned-50" ]]
    [ "$status" -eq 0 ]
}
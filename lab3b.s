## Daniel J. Ellard -- 02/21/94
## palindrome.asm -- read a line of text and test if it is a palindrome.
##
## A palindrome is a string that is exactly the same when read backwards
## as when it is read forward (e.g. anna).
##
## Whitespace is considered significant in this version, so "ahahaha"
## is a palindrome while "aha haha" is not.
##
## Register usage:
## $t1 - A - initially, the address of the first character in the string S.
## $t2 - B - initially, the address of the the last character in S.
## $t3 - the character at address A.
## $t4 - the character at address B.
## $v0 - syscall parameter / return values.
## $a0 - syscall parameters.
## $a1 - syscall parameters.


.data
is_palin_msg: .asciiz "The string is a palindrome.\n"
not_palin_msg: .asciiz "The string is not a palindrome.\n"
string_space: .space 1024 # reserve 1024 bytes for the string.

.text
main: #
## read the string S:
	la $30, string_space #
    nop
	li $31, 1024 #
    nop
	li $29, 8 # load "read_string" code into $v0.
    nop
	syscall #
    nop

	la $0, string_space #
    nop
	la $1, string_space #
    nop

length_loop: # length of the string
	lb $2, ($1) # load the byte at addr B into $t3.
    nop
	beqz $2, end_length_loop # if $t3 == 0, branch out of loop.
    nop

	addi $1, $1, 1 # otherwise, increment B,
	b length_loop # and repeat the loop.
    nop

end_length_loop:
	subi $1, $1, 2 # subtract 2 to move B back past

# the '\0' and '\n'.

test_loop:
	bge $0, $1, is_palin # if A >= B, it's a palindrome.
    nop

	lb $2, ($0) # load the byte at addr A into $t3,
    nop
	lb $3, ($1) # load the byte at addr B into $t4.
    nop
	bne $2, $3, not_palin # if $t3 != $t4, not a palindrome.
    nop

# Otherwise,

	addi $0, $0, 1 # increment A,
	subi $1, $1, 1 # decrement B,
	b test_loop # and repeat the loop.
    nop

is_palin: # print the is_palin_msg, and exit.

	la $30, is_palin_msg #
    nop
	li $29, 4 #
    nop
	syscall #
	b exit #
    nop

not_palin: #
	la $30, not_palin_msg # print the not_palin_msg, and exit.
    nop
	li $29, 4 #
    nop
	syscall #
	b exit #
    nop

exit: # exit the program
	li $29, 10 # load "exit" code into $v0.
    nop
	syscall # make the system call.

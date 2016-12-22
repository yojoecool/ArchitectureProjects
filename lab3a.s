
	.text

main:
    li $1, 0
    nop
	li $2, 32
    nop

loop:
    subi $2, $2, 1
	bge  $2, $1, loop
	nop

	li $29, 10
    nop
	syscall
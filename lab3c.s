
	.text

main:
    li $0, 0
    nop
    li $1, 0
    nop
	li $2, 10
    nop
	li $3, 100
    nop
	li $4, 1000
    nop
	li $5, 7
    nop

loop:
    subi $5, $5, 1
	add $1, $1, $2
	add $1, $1, $3
	add $1, $1, $4
	bge $5, $0, loop
    nop

	add $4, $0, $1
	li $29, 1
    nop
	syscall

	li $29, 10
    nop
	syscall
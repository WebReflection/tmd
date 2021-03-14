# Simply create a tmd executable

all:
	gcc -Os -o tmd tmd.c

test:
	clear
	make
	cat README.md | ./tmd && echo 'Test Passed 👍'

style:
	style50 tmd.c

heap:
	valgrind ./tmd README.md

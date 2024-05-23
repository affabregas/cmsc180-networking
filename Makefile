port = 77783
n = 30000
no_of_cli = 4

ser:
	gcc p5_root.c -pthread -lm
	./a.out $(n) $(port) 0 $(no_of_cli)
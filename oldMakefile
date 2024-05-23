port = 77782
n = 30000
no_of_cli = 2

ser:
	gcc p4.c -pthread -lm
	./a.out $(n) $(port) 0 $(no_of_cli)

cli:
	gcc -o hi.out p4.c -pthread -lm
	./a.out $(n) $(port) 1
	
c1:
	gcc p4.c -pthread -lm
	./a.out $(n) $(port) 1 2
	
c2:
	gcc p4.c -pthread -lm
	./a.out $(n) $(port) 1 1 &
	./a.out $(n) $(port) 1 1 &

c4:
	gcc p4.c -pthread -lm
	./a.out $(n) $(port) 1 1 &
	./a.out $(n) $(port) 1 1 &
	./a.out $(n) $(port) 1 1 &
	./a.out $(n) $(port) 1 1 &

t2:
	gcc p4.c -pthread -lm
	./a.out $(n) $(port) 0 2 &
	./a.out $(n) $(port) 1 2 &
	./a.out $(n) $(port) 1 2 &

t4:
	gcc p4.c -pthread -lm
	./a.out $(n) $(port) 0 4 &
	./a.out $(n) $(port) 1 4 &
	./a.out $(n) $(port) 1 4 &
	./a.out $(n) $(port) 1 4 &
	./a.out $(n) $(port) 1 4 &

t8:
	gcc p4.c -pthread -lm
	./a.out $(n) $(port) 0 8 &
	./a.out $(n) $(port) 1 8 &
	./a.out $(n) $(port) 1 8 &
	./a.out $(n) $(port) 1 8 &
	./a.out $(n) $(port) 1 8 &
	./a.out $(n) $(port) 1 8 &
	./a.out $(n) $(port) 1 8 &
	./a.out $(n) $(port) 1 8 &
	./a.out $(n) $(port) 1 8 &

t16:
	gcc p4.c -pthread -lm
	./a.out $(n) $(port) 0 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &
	./a.out $(n) $(port) 1 16 &

	
	

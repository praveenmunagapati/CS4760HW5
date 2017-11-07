all : oss user
clean :
	rm oss.o user.o msglog.out oss user
oss : oss.o
	gcc -pthread -g -o oss oss.o
oss.o : oss.c
	gcc -c -g oss.c
user : user.o
	gcc -pthread -g -o user user.o
user.o : user.c
	gcc -c -g user.c

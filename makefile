client.exe: client.o
	gcc -o client.exe client.o

client.o: client.c 
	gcc -c client.c

serveur.exe: serveur.o
	gcc -o serveur.exe serveur.o

serveur.o: serveur.c 
	gcc -c serveur.c

clean:
	rm *.o
	rm *.exe

cleanTxt:
	rm *.txt

all: serveur.exe client.exe cleanTxt

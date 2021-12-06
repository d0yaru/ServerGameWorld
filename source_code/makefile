server: main.o timerclass.o entityclass.o zoneclass.o networkclass.o systemclass.o
	g++ -o server main.o timerclass.o entityclass.o zoneclass.o networkclass.o systemclass.o -pthread

main.o: main.cpp
	g++ -c main.cpp

timerclass.o: timerclass.cpp
	g++ -c timerclass.cpp

entityclass.o: entityclass.cpp
	g++ -c entityclass.cpp

zoneclass.o: zoneclass.cpp
	g++ -c zoneclass.cpp

networkclass.o: networkclass.cpp
	g++ -c networkclass.cpp

systemclass.o: systemclass.cpp
	g++ -c systemclass.cpp

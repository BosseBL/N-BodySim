CC=g++
ifdef gui 
	CFLAGS = -I. -std=c++11 -fopenmp -D _GUI
	DEPS = physics.h sandbox.h graphics.h
	OBJ = main.o physics.o sandbox.o graphics.o
	LIBS=-lsfml-graphics -lsfml-system -lsfml-window -lm
else
	CFLAGS = -I. -std=c++11 -fopenmp
	DEPS = physics.h sandbox.h
	OBJ = main.o physics.o sandbox.o
	LIBS= -lm
endif



%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

nbody: $(OBJ)
	g++ $(CFLAGS) -o $@ $^ $(LIBS)
	

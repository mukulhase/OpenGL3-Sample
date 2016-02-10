all: game

game: 201401144_game.cpp glad.c
	g++ -o game 201401144_game.cpp glad.c -framework OpenGL -lglfw

clean:
	rm game

CFLAGS=-Wall -g `sdl-config --cflags --libs --static-libs` -lSDL_Image -lSDL_TTF

game: object.o gameobjects.o

clean:
	rm -f object.o gameobjects.o game

Snake: Snake.c
	cc Snake.c -o Snake \
	-I/opt/homebrew/opt/ncurses/include \
	-L/opt/homebrew/opt/ncurses/lib \
	-lncurses -g

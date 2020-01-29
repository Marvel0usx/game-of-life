#include <stdio.h>
#include <stdlib.h>

#include "life_helpers.h"


int main(int argc, char **argv) {
	argc = 4;
    if (argc != 4) {
        fprintf(stderr, "Usage: life2D rows cols states\n");
        return 1;
    }

    // Initializing board
	int height = strtol(argv[1], NULL, 10);
	int width = strtol(argv[2], NULL, 10);
	int rounds = strtol(argv[3], NULL, 10);
    int **board = malloc(height * sizeof(int *));
    
    for (int **p = board; p < (board + height); p++)
        *p = malloc(width * sizeof(int));

    int tmp;
    for (int **p = board; p < (board + height); p++) {
        for (int i = 0; i < width; i++) {
            if (scanf("%d", &tmp) != EOF)
				*(*p + i) = tmp;
        }
    }

	// Linearize board
	int *lin_board = malloc(height * width * sizeof(int));
	int idx = 0;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			lin_board[idx++] = board[i][j] ;
		}
	}

	// mainloop
	for (int i = 0; i < rounds; i++) {
		print_state(lin_board, height, width);
		update_state(lin_board, height, width);
	}

	// clean-up
	free(lin_board);
	for (int **p = board; p < (board + height); p++)
		free(*p);
	free(board);
}

#include <stdio.h>
#include <stdlib.h>

static int num_neighbor(int *board, int c, int i);

void print_state(int *board, int num_rows, int num_cols) {
	for (int i = 0; i < num_rows * num_cols; i++) {
		printf("%d", board[i]);
		if (((i + 1) % num_cols) == 0) {
			printf("\n");
		}
	}
	printf("\n");
}

void update_state(int *board, int num_rows, int num_cols) {
	int *n = &num_cols;
	int *m = &num_rows;

	for (int i = 0; i < num_cols * num_rows; i++) {
		// cells that are at borders
		if (0 <= i && i <= *n - 1) continue;
		if (i % *n == 0 || i % *n == *n - 1) continue;
		if ((*m + *n) * *n <= i && i <= *m * *n - 1) continue;
		// cells that are guaranteed to have neighbor
		int n_neighbor = num_neighbor(board, num_cols, i);
		if (board[i] && (n_neighbor < 2 || n_neighbor > 3)) {
			board[i] = 0;
		}
		if (!board[i] && (n_neighbor == 2 || n_neighbor == 3)) {
			board[i] = 1;
		}
	}
	return 0;
}

static int num_neighbor(int *board, int c,  int i) {
	int count = 0;
	int idxs[] = { i - c - 1, i - c, i - c + 1, i - 1, i + 1, i + c - 1, i + c, i + c + 1 };
	for (int *p = idxs; p < (idxs + 8); p++) {
		count += board[*p];
	}
	return count;
}

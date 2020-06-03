#include <stdio.h>
#include <stdlib.h>
#include "life_helpers.h"

static int num_neighbor(int **map, int r, int c);

/*
 * Update map according to the following rules:
 * 	- Any live cell with fewer than two live neighbours dies, as if by underpopulation.
 *	- Any live cell with more than three live neighbours dies, as if by overpopulation.
 *	- Any live cell with two or three live neighbours lives on to the next generation.
 *	- Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
 *  - Any cell at the border does not change.
 */
void update_map(int **map, int nrows, int ncols) {
	int n_n;
	/* Update is based on old map solely */
	int **ol_m = malloc(sizeof(int **));
	for (int row = 0; row < nrows; row++) {
		ol_m[row] = malloc(sizeof(int *));
		for (int col = 0; col < ncols; )
			ol_m[row][col++] = map[row][col];
	}

	for (int row = 0; row < nrows; row++) {
		for (int col = 0; col < ncols; col++) {
			if (row == 0 || row == nrows - 1)
				continue;
			if (col == 0 || col == ncols - 1)
				continue;
			n_n = num_neighbor(map, row, col);
			if (ol_m[row][col] && (n_n < 2 || n_n > 3)) {
				map[row][col] = 0;
			} else if (!ol_m[row][col] && n_n == 3) {
				map[row][col] = 1;
			}
		}
	}

	for (int row = 0; row < nrows; row++) {
		free(ol_m[row]);
	}
	free(ol_m);
}

static int num_neighbor(int **map, int r, int c) {
	int count = 0;
	for (int **row = map + r - 1; row - map < r + 1; row++) {
		for (int *col = *row + c - 1; col - row < c + 1; col++) {
			count += *col;
		}
	}
	return count;
}

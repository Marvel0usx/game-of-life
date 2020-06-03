#include <stdlib.h>

static int num_neighbor(int *map, int c,  int i) {
	int count = 0;
	int idxs[] = { i - c - 1, i - c, i - c + 1, i - 1, i + 1, i + c - 1, i + c, i + c + 1 };
	for (int *p = idxs; p < (idxs + 8); p++) {
		count += map[*p];
	}
	return count;
}

/*
 * Update map according to the following rules:
 * 	- Any live cell with fewer than two live neighbours dies, as if by underpopulation.
 *	- Any live cell with more than three live neighbours dies, as if by overpopulation.
 *	- Any live cell with two or three live neighbours lives on to the next generation.
 *	- Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
 *  - Any cell at the border does not change.
 */
void update_map(int *map, int nrow, int ncol) {
	int *ol_m = malloc(sizeof(int) * nrow * ncol);
	for (int idx = 0; idx < nrow * ncol; ) {
		ol_m[idx++] = map[idx];
	}

	for (int i = 0; i < ncol * nrow; i++) {
		// cells that are at borders
		if (0 <= i && i <= ncol - 1) continue;
		if (i % ncol == 0 || i % ncol == ncol - 1) continue;
		if ((nrow + ncol) * ncol <= i && i <= nrow * ncol - 1) continue;
		// cells that are guaranteed to have neighbor
		int n_neighbor = num_neighbor(ol_m, ncol, i);
		if (ol_m[i] && (n_neighbor < 2 || n_neighbor > 3)) {
			map[i] = 0;
		}
		if (!ol_m[i] && (n_neighbor == 3)) {
			map[i] = 1;
		}
	}
	free(ol_m);
	return 0;
}

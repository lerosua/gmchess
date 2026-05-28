/*
 * Shared non-GTK types for gmchess core code.
 */

#ifndef GMCHESS_CORE_TYPES_H
#define GMCHESS_CORE_TYPES_H

struct ChessPoint {
	int x;
	int y;

	ChessPoint() : x(0), y(0) {}
	ChessPoint(int x_, int y_) : x(x_), y(y_) {}
};

#endif /* GMCHESS_CORE_TYPES_H */

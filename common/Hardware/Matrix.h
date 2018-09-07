/*
 * Matrix.h
 *
 *  Created on: Aug 23, 2018
 *      Author: shahada
 */

#ifndef COMMON_HARDWARE_MATRIX_H_
#define COMMON_HARDWARE_MATRIX_H_

/** @brief Manages a Matrix Keypad Inputs
 *
 * 	For now let's assume that there can only be one Matrix in any
 * 	hardware. Reason being that for any given number of i/o lines,
 * 	you always get more switches by making them into one huge
 * 	matrix as opposed to multiple smaller ones.
 *
 * 	A bigger limitation is that we only allow the i/o lines in a
 * 	row or column to come from the same interface, and they must
 * 	be in sequence. This is because we can read 16-bits off a
 * 	MCP23017 in one i2c read, so we might a well use it.
 *
 * 	If someone wants to make a matrix with Pi GPIO Lines, then
 * 	this needs some re-thinking, as the way the lines are
 * 	numbered it's not easy to get a sequence. The same goes for
 * 	using more than 16 i/o lines in a row or column. Someday we
 * 	may want to support multiple matrices, and non-sequential
 * 	i/o lines in rows and columns.
 *
 * 	Leaving that for another day.
 *
 * 	Update: I've split the implementation into MatrixSequential
 * 	and MatrixNonSequential. Sequential makes use of
 * 	GPIO::readInputSequence which lets the underlying hardware
 * 	determine how to do sequential reads. NonSequential is not
 * 	implemented yet but gives an idea of how I plan to solve
 * 	this issue.
 *
 *
 */

#include <vector>

#include "GPIOPin.h"

namespace Hardware {

class Matrix {
protected:

	// @brief first switch number to use
	int switchStart;

	// @brief size of matrix.
	int colSize;
	int rowSize;


	// @brief previous state of each switch [row][col](true=cn)
	std::vector< std::vector <bool>> prevState;

	enum class THREADSTATE { RUNNING, STOPPING, STOPPED } threadState;

public:
	Matrix (int switchStart) {
		this->switchStart = switchStart;
		threadState = THREADSTATE::STOPPED;

		// subclass constructor will overwrite these
		colSize = 0;
		rowSize = 0;
	}

	void initPrevState (int rows, int cols);

	virtual ~Matrix ();

	int getSwitchStart ();
	int getColSize();
	int getRowSize();

};

}
#endif /* COMMON_HARDWARE_MATRIX_H_ */

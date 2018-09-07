/*
 * Matrix.cpp
 *
 *  Created on: Aug 23, 2018
 *      Author: shahada
 */

#include "Matrix.h"

namespace Hardware {

Matrix::~Matrix() {
	// TODO Auto-generated destructor stub
}

void Matrix::initPrevState (int rows, int cols) {

	prevState.resize (rows);
	for (int r = 0; r < rows; r++) {
		prevState[r].resize(cols);
	}
}

} /* namespace configFiles */

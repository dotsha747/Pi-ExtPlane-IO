/*
 * MatrixNonSequential.cpp
 *
 *  Created on: Aug 23, 2018
 *      Author: shahada
 */

#include "MatrixNonSequential.h"

namespace Hardware {

MatrixNonSequential::MatrixNonSequential(int switchStart, std::vector<GPIOPin> cols,
		std::vector<GPIOPin> rows) : Matrix (switchStart) {

	this->cols = cols;
	this->rows = rows;


}

MatrixNonSequential::~MatrixNonSequential() {
	// TODO Auto-generated destructor stub
}

} /* namespace Hardware */

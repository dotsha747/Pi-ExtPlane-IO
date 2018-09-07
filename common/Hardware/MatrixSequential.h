/*
 * MatrixSequential.h
 *
 *  Created on: Aug 23, 2018
 *      Author: shahada
 */

#ifndef COMMON_HARDWARE_MATRIXSEQUENTIAL_H_
#define COMMON_HARDWARE_MATRIXSEQUENTIAL_H_

#include "Matrix.h"



namespace Hardware {

/** @brief implementation of a sequential matrix
 *
 * 	A sequential matrix is one where the pins that make up the rows and cols
 * 	each come from the same provider address, and are in sequence, and there
 * 	are < 16 pins in the row or column.
 *
 * 	It's an optimized matrix used to read rows and columns off a MCP23017,
 * 	where readng 16 pins can be done in a single i2c operation.
 *
 */
class MatrixSequential: public Matrix {
protected:

	int colInputAddr;
	int colStartPin;
	int colEndPin;
	int rowOutputAddr;
	int rowStartPin;
	int rowEndPin;

public:
	MatrixSequential(int switchStart, int colInputAddr, int colStartPin,
			int colEndPin, int rowOutputAddr, int rowStartPin, int rowEndPin);
	virtual ~MatrixSequential();

	void scanLoop ();
};

} /* namespace Hardware */

#endif /* COMMON_HARDWARE_MATRIXSEQUENTIAL_H_ */

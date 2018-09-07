/*
 * MatrixNonSequential.h
 *
 *  Created on: Aug 23, 2018
 *      Author: shahada
 */

#ifndef COMMON_HARDWARE_MATRIXNONSEQUENTIAL_H_
#define COMMON_HARDWARE_MATRIXNONSEQUENTIAL_H_

#include <vector>

#include "Matrix.h"
#include "GPIOPin.h"

namespace Hardware {

class MatrixNonSequential : public Matrix{
protected:
	std::vector<GPIOPin> cols;
	std::vector<GPIOPin> rows;
public:
	MatrixNonSequential(int switchStart, std::vector<GPIOPin> cols,
			std::vector<GPIOPin> rows);
	virtual ~MatrixNonSequential();
};

} /* namespace Hardware */

#endif /* COMMON_HARDWARE_MATRIXNONSEQUENTIAL_H_ */

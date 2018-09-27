/**
 * I2C.h
 *
 *  Created on: Apr 4, 2016
 *      Author: shahada
 */

#ifndef PIEXTPLANE_SRC_I2C_H_
#define PIEXTPLANE_SRC_I2C_H_

/** @brief Communicate with i2c slave device

Allows reading and writing of 8 and 16 bit words to i2c slave device registers.

*/

class I2C {
private:
	int bus;
	int i2caddr;

	int i2cdev;

public:

	enum class ENDIAN { LITTLE=0 , BIG };


	/**@brief constructor initialize with i2caddr of slave and bus number.
	   * bus number should be 0 on first version Pis, and 1 on later models.
	   * set bus to -1 to autodetect.
	*/
	
	I2C(int i2caddr, int bus = -1);
	virtual ~I2C();

	void init ();

	uint8_t readRaw ();
	void writeRaw (uint8_t data);

	/** @brief read a byte from a register
	 *
	 */
	void writeReg8 (uint8_t reg, uint8_t data);

	/** @brief write a byte to a register
	 *
	 */

	uint8_t readReg8 (uint8_t reg);

	/**@brief read a 16-bit word from a register.
	 *
	 * SMBUS standard is to use low endian words.
	 *
	 */
	uint16_t readReg16 (uint8_t reg, ENDIAN e = ENDIAN::LITTLE);



	/** @brief write a 16-bit word to a register.
	 *
	 * SMBUS standard is to use low endian words.
	 */
	void writeReg16 (uint8_t reg, uint16_t data, ENDIAN e = ENDIAN::LITTLE);


	/** @brief writes a 16-bit word to a register, big endian.
	 *
	 * SMBUS standard is to use low endian words. You probably want
	 * writeReg16 instead.
	 */


	int readRegBlock (uint8_t reg, uint8_t * data, int count);


	/** @brief write a block of (up to 32) bytes beginning from a register
	 *
	 */

	int writeRegBlock (uint8_t reg, uint8_t * data, int count);

};

#endif /* PIEXTPLANE_SRC_I2C_H_ */

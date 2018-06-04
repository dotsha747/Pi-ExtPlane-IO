#include <iostream>
#include <iomanip>
#include <unistd.h>

#include "MCP3208.h"

using namespace std;

int main(int argc, char * argv[]) {

	MCP3208 * mcp3208 = new MCP3208(0, 0);

	while (true) {

		cout << "\r";

		for (int port = 0; port < 8; port++) {

			double v = mcp3208->AnalogRead(port, true) / 4096.0 * 3.3;

			cout << "Ch" << port << ":" << fixed << setfill('0') << setw(2)
					<< setprecision(3) << v << " ";
			cout.flush();

		};

		usleep(500000); // 500 m/s

	}

}

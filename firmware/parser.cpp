#include <iostream>
#include <fstream>
#include <iomanip>

int main () {
    std::ifstream Dump("dump.log");
    std::string line;

    const std::string frameId = "705";

    while (getline(Dump, line)) {
        // get ID
        if (line.substr(26, 3).compare(frameId) != 0) {
            continue;
        }

        //grab timestamp
        std::cout << line.substr(0, 19) << ": WheelSpeedFR: ";

        std::string data = line.substr(30, 16);
        uint64_t dataInInt;

        sscanf(data.c_str(), "%lx", &dataInInt);

        // 16 bits long starting from position 0, little endian
        // grab 1 byte each
        int mask = 0xFF;
        uint msb = (dataInInt >> 48) & mask;
        uint fr = msb << 8;
        uint lsb = (dataInInt >> 56) & mask;
        fr |= lsb;

        msb = (dataInInt >> 16) & mask;
        uint rr = msb << 8;
        lsb = (dataInInt >> 24) & mask;
        rr |= lsb;

        std::cout << std::fixed << std::setprecision(1);

        double frOutput = fr * 0.1;
        std::cout << frOutput << '\n';

        double rrOutput = rr * 0.1;
        std::cout << line.substr(0, 19) << ": WheelSpeedRR: " << rrOutput << '\n';
    }

    Dump.close();
}
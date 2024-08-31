#include <iostream>
#include <fstream>
#include <iomanip>

int main () {
    std::ifstream Dump("dump.log");
    std::string line;

    const std::string frameId = "705";

    while (getline(Dump, line)) {
        // get ID
        if (line.substr(26, 3) != frameId) {
            continue;
        }

        // get timestamp substring
        std::cout << line.substr(0, 19) << ": WheelSpeedFR: ";
        // data payload substring
        std::string data = line.substr(30, 16);
        uint64_t dataInInt;
        sscanf(data.c_str(), "%lx", &dataInInt);

        // 16 bits long starting from position 0, little endian
        int mask = 0xFF;
        // Shift data by 48 bits to grab 2nd byte
        uint msb = (dataInInt >> 48) & mask;
        uint wheelSpeedFRVal = msb << 8;
        // grab first byte
        uint lsb = (dataInInt >> 56) & mask;
        wheelSpeedFRVal |= lsb;

        // 16 bits long starting from position 32, little endian
        // grab 2nd byte
        msb = (dataInInt >> 16) & mask;
        uint wheelSpeedRRVal = msb << 8;
        // grab 1st byte
        lsb = (dataInInt >> 24) & mask;
        wheelSpeedRRVal |= lsb;

        // set to 1 DP
        std::cout << std::fixed << std::setprecision(1);

        double frOutput = wheelSpeedFRVal * 0.1;
        std::cout << frOutput << '\n';

        double rrOutput = wheelSpeedRRVal * 0.1;
        std::cout << line.substr(0, 19) << ": WheelSpeedRR: " << rrOutput << '\n';
    }

    Dump.close();
    return 0;
}
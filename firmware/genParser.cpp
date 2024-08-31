#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>
#include <tuple>
#include <vector>
#include <sstream>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: ./genParser <DBC file path> <CAN data file path>\n";
        exit(1); 
    }
    // map to store CAN frames
    std::map<uint32_t, std::vector<std::vector<std::string>>> mp;

    // DBC Parsing
    std::ifstream Dbc(argv[1]);
    std::string line;

    while (getline(Dbc, line)) {
        if (line.substr(0, 3) != "BO_") {
            continue;
        }
        // Grab CAN frame ID
        std::size_t firstSpace = line.find(' ');
        std::size_t secondSpace = line.find(' ', firstSpace + 1);
        int idInt = std::stoi(line.substr(firstSpace + 1, secondSpace - firstSpace - 1));
        
        // Store all signals in array
        std::vector<std::vector<std::string>> signals;
        while (getline(Dbc, line)) {
            // Separate string by whitespace
            std::istringstream sstream(line);
            std::vector<std::string> strings;
            std::string str;

            while (sstream >> str) {
                strings.push_back(str);
            }

            if (strings.size() == 0 || strings[0] != "SG_") {
                break;
            }

            signals.push_back(strings);
        }

        mp[idInt] = signals;
    }

    // CAN Parser
    std::ifstream Dump(argv[2]);

    while (getline(Dump, line)) {
        // split into timestamp, interface, id#payload
        std::istringstream sstream(line);
        std::vector<std::string> logStrings;
        std::string str;

        while (sstream >> str) {
            logStrings.push_back(str);
        }

        // get ID
        std::size_t hash = logStrings[2].find('#');
        std::string canIdString = logStrings[2].substr(0, hash);
        // convert ID string to int
        uint32_t idInt;
        sscanf(canIdString.c_str(), "%x", &idInt);

        // ignore log if no DBC entry with same ID exists
        if (!mp.count(idInt)) {
            continue;
        }

        // Grab payload string and convert to 64 bit int
        uint64_t payload;
        std::string payloadString = logStrings[2].substr(hash + 1, logStrings[2].length() - (hash + 1));
        sscanf(payloadString.c_str(), "%lx", &payload);

        // iterate through every signal in CAN frame
        std::vector<std::vector<std::string>> signals = mp[idInt];
        for (std::vector<std::string> signal : signals) {
            // signal[3] contains start bit, bit length, endianess and sign info
            std::size_t atPos = signal[3].find('@');
            std::string endianess = signal[3].substr(atPos, 2);
            std::string signage = signal[3].substr(atPos + 2, 1);

            std::size_t pipePos = signal[3].find('|');
            int startBit = std::stoi(signal[3].substr(0, pipePos));
            int bitLen = std::stoi(signal[3].substr(pipePos + 1, atPos - pipePos - 1));

            // create bitmask with bitLen Length
            uint64_t mask = 0;
            for (int i = 0; i < bitLen; i++) {
                mask |= 1;
                if (i != bitLen - 1) {
                    mask <<= 1;
                }
            }
            // shift bitmask to correct pos
            int shiftPos = 64 - startBit - bitLen;
            mask <<= shiftPos;

            // grab bits
            uint64_t data = payload & mask;

            // shift to rightmost pos
            data >>= shiftPos;
            uint64_t actualData = data;

            if (endianess == "@1") {
                // little endian, reverse byte order
                uint8_t oneByteMask = 0xFF;
                actualData = 0;
                for (int i = 0; i < bitLen / 8; i++) {
                    actualData |= data & oneByteMask;
                    data >>= 8;
                    if (i != (bitLen / 8) - 1) {
                        actualData <<= 8;
                    }
                }
            }
            // leave big endian alone

            // signal[4] contains factor and offset info
            std::size_t commaPos = signal[4].find(',');
            double factor = std::stof(signal[4].substr(1, commaPos - 1));
            std::size_t closingBracketPos = signal[4].find(')');
            double offset = std::stof(signal[4].substr(commaPos + 1, closingBracketPos - commaPos - 1));
            
            double output;
            if (signage == "-") {
                // if signed
                int signedData = static_cast<int64_t>(actualData);
                output = offset + signedData * factor;
            } else {
                output = offset + actualData * factor;
            }

            std::cout << std::fixed << std::setprecision(1);
            std::cout << logStrings[0] << ": " << signal[1] << ": " << output << '\n';
        }
    }

    Dump.close();
    Dbc.close();
    return 0;
}
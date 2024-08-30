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

    std::map<uint32_t, std::vector<std::vector<std::string>>> mp;

    // DBC Parsing
    std::ifstream Dbc(argv[1]);
    std::string line;

    while (getline(Dbc, line)) {
        if (line.substr(0, 3) != "BO_") {
            continue;
        }

        std::size_t firstSpace = line.find(' ');
        std::size_t secondSpace = line.find(' ', firstSpace + 1);

        int idInt = std::stoi(line.substr(firstSpace + 1, secondSpace - firstSpace - 1));

        std::vector<std::vector<std::string>> signals;
        while (getline(Dbc, line)) {
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
        std::vector<std::string> strings;
        std::string str;

        while (sstream >> str) {
            strings.push_back(str);
        }

        // get ID
        std::size_t hash = strings[2].find('#');
        std::string canIdString = strings[2].substr(0, hash);

        uint32_t idInt;
        sscanf(canIdString.c_str(), "%x", &idInt);

        if (!mp.count(idInt)) {
            continue;
        }

        uint64_t payload;
        std::string payloadString = strings[2].substr(hash + 1, strings[2].length() - (hash + 1));
        sscanf(payloadString.c_str(), "%lx", &payload);

        std::vector<std::vector<std::string>> sig = mp[idInt];
        for (std::vector<std::string> s : sig) {
            std::size_t atPos = s[3].find('@');
            std::string endianess = s[3].substr(atPos, 2);
            std::string signage = s[3].substr(atPos + 2, 1);

            std::size_t pipePos = s[3].find('|');
            int startBit = std::stoi(s[3].substr(0, pipePos));
            int bitLen = std::stoi(s[3].substr(pipePos + 1, atPos - pipePos - 1));

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
                // little endian
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

            std::size_t commaPos = s[4].find(',');
            double factor = std::stof(s[4].substr(1, commaPos - 1));
            std::size_t closingBracketPos = s[4].find(')');
            double offset = std::stof(s[4].substr(commaPos + 1, closingBracketPos - commaPos - 1));
            
            double output;
            if (signage == "-") {
                // if signed
                int signedData = static_cast<int64_t>(actualData);
                output = offset + signedData * factor;
            } else {
                output = offset + actualData * factor;
            }

            std::cout << std::fixed << std::setprecision(1);
            std::cout << strings[0] << ": " << s[1] << ": " << output << '\n';
        }
    }

    return 0;
}
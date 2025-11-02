#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <filesystem>

using namespace std;

// Define a custom structure to represent a 512-bit big integer, using 32-bit (4-byte) elements as the base units.
class BigInt {
    private:
        vector<uint32_t> num;
    public:
        BigInt (); //default = 0
        BigInt (uint64_t number){
            if (number == 0){
                return;
            }
            while (number) {
                this->num.push_back(number & 0xffffffffu);
                number >> 32;
            }
        }


};
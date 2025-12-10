#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

#include <iomanip>

using namespace std;

class BigInt
{
private:
    vector<uint32_t> num;

    // remove all bit 0 at the end
    void norm()
    {
        while (num.size() > 1 && num.back() == 0)
        {
            num.pop_back();
        }
    }

public:

    BigInt()
    {
        num.push_back(0);
    }; // default = 0
    BigInt(uint64_t number)
    {
        if (number == 0)
        {
            num.push_back(0);
        }
        while (number)
        {
            this->num.push_back(number & 0xffffffffu);
            number >>= 32;
        }
    }

    void displayNum() const {
    cout << "0x";
    // In từ block cao nhất đến thấp nhất (thường dễ đọc hơn)
    for (int i = num.size() - 1; i >= 0; i--) {
        // In mỗi block với 8 chữ số hexa, có thêm các số 0 ở đầu nếu thiếu
        cout << hex << setw(8) << setfill('0') << uppercase << num[i];
    }
    cout << dec << endl; // chuyển lại sang hệ 10 để tránh ảnh hưởng về sau
    }

    void displayNumBin() const {
        if (num.empty()) { cout << "0b0\n"; return; }
        cout << "0b";

        bool started = false;
        for (int i = (int)num.size() - 1; i >= 0; --i) {
            for (int bit = 31; bit >= 0; --bit) {
                bool bitValue = (num[i] >> bit) & 1u;
                if (bitValue) started = true;
                if (started) cout << bitValue;
            }
        }
        if (!started) cout << '0'; // nếu toàn là 0
        cout << '\n';
    }

    void displayNumDec() const {
    if (num.empty()) {
        cout << "0\n";
        return;
    }

    // Tạo 1 bản copy để chia dần ra cơ số 10
    vector<uint32_t> temp = num;
    vector<uint8_t> digits; // lưu chữ số thập phân (ngược)

    while (!(temp.size() == 1 && temp[0] == 0)) {
        uint64_t carry = 0;
        // Chia temp cho 10, giống như chia tay lớn
        for (int i = (int)temp.size() - 1; i >= 0; --i) {
            uint64_t cur = (carry << 32) + temp[i];
            temp[i] = static_cast<uint32_t>(cur / 10);
            carry = cur % 10;
        }

        digits.push_back(static_cast<uint8_t>(carry));

        // Bỏ block 0 ở cuối
        while (temp.size() > 1 && temp.back() == 0)
            temp.pop_back();
    }

    // In ngược lại (vì digits chứa từ LSD → MSD)
    for (int i = (int)digits.size() - 1; i >= 0; --i)
        cout << char('0' + digits[i]);
    cout << '\n';
}




    vector<uint32_t> getData (){return this->num;}
    size_t bitLenght () const {
        size_t bitCnt = (this->num.size() - 1) * 32;
        if (this->num.empty()){
            return 0;
        }
        uint32_t lastBlock = this->num.back();
        while (lastBlock){
            bitCnt++;
            lastBlock >>= 1;
        }
        return bitCnt;
    }

    static BigInt hexToBig(const string &hexStr) {
        BigInt res;
        res.num.clear();
        string s = hexStr;
        string clean;
        for (char c : s)
            if (!isspace(c))
                clean.push_back(c);

        if (clean.size() % 8 != 0)
            clean.append(8 - clean.size() % 8, '0');

        for (size_t i = 0; i < clean.size(); i += 8) {
            string part = clean.substr(i, 8);
            uint32_t val = 0;
            for (int j = 0; j < 8; j++) {
                char ch = part[j];
                val |= (uint32_t)((ch >= 'A' ? ch - 'A' + 10 : ch - '0')) << (4 * j);
            }
            res.num.push_back(val);
        }
        res.norm();
        return res;
    }

    string bigToHex() const {
        string s;
        for (uint32_t block : num) {
            for (int i = 0; i < 8; i++) {
                uint32_t nibble = (block >> (4 * i)) & 0xF;
                s.push_back(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
            }
        }
        while (s.size() > 1 && s.back() == '0')
            s.pop_back();
        return s;
    }

    // Math operator
    BigInt operator+(const BigInt &other) const
    {
        BigInt result;
        result.num.clear();

        uint64_t carry = 0;
        size_t bitSize = max(this->num.size(), other.num.size());

        size_t index = 0;
        while (index < bitSize || carry)
        {
            uint64_t num1 = (index < this->num.size() ? this->num[index] : 0);
            uint64_t num2 = (index < other.num.size() ? other.num[index] : 0);

            uint64_t sum = num1 + num2 + carry;
            result.num.push_back(sum & 0xffffffffu);
            carry = sum >> 32;
            index++;
        }

        result.norm();
        return result;
    }
    BigInt operator-(const BigInt &other) const
    {
        BigInt result = *this;
        uint64_t carry = 0;

        for (size_t i = 0; i < this->num.size(); i++)
        {

            uint64_t num1 = this->num[i];
            uint64_t num2 = (i < other.num.size() ? other.num[i] : 0);
            uint64_t temp = 0;

            if (num1 < num2 + carry)
            {
                temp = (1ull << 32) + num1 - num2 - carry;
                carry = 1;
            }
            else
            {
                temp = num1 - num2 - carry;
                carry = 0;
            }
            result.num[i] = static_cast<uint32_t>(temp);
        }
        result.norm();
        return result;
    }
    BigInt operator*(const BigInt &other) const
    {
        BigInt result;
        result.num.assign(num.size() + other.num.size(), 0);

        for (size_t i = 0; i < num.size(); ++i)
        {
            uint64_t carry = 0;
            for (size_t j = 0; j < other.num.size() || carry; ++j)
            {
                uint64_t cur = result.num[i + j] +
                               uint64_t(num[i]) * (j < other.num.size() ? other.num[j] : 0) + carry;

                result.num[i + j] = cur & 0xffffffffu;
                carry = cur >> 32;
            }
        }

        result.norm();
        return result;
    }
    BigInt operator/(const BigInt &other) const
    {
        BigInt quotient(0);
        BigInt remainder(0);

        size_t totalBits = this->num.size() * 32;

        for (int i = totalBits - 1; i >= 0; --i)
        {
            remainder = remainder.shiftLeft(1);

            if ((this->num[i / 32] >> (i % 32)) & 1)
            {
                remainder.num[0] |= 1;
            }

            if (!(remainder < other))
            {
                remainder = remainder - other;

                size_t block = i / 32;
                size_t bit = i % 32;

                if (block >= quotient.num.size())
                {
                    quotient.num.resize(block + 1, 0);
                }

                quotient.num[block] |= (1u << bit);
            }
        }

        quotient.norm();
        return quotient;
    }
    BigInt &operator++()
    {
        uint64_t carry = 1;
        for (size_t i = 0; i < num.size() && carry; ++i)
        {
            uint64_t cur = (uint64_t)num[i] + carry;
            num[i] = cur & 0xffffffffu;
            carry = cur >> 32;
        }
        if (carry)
            num.push_back(carry);
        return *this;
    }

    BigInt &operator--()
    {
        size_t i = 0;
        while (i < num.size())
        {
            if (num[i] > 0)
            {
                num[i]--;
                break;
            }
            else
            {
                num[i] = 0xffffffffu;
                i++;
            }
        }
        norm();
        return *this;
    }
    BigInt shiftLeft(size_t bits) const
    {
        BigInt result = *this;
        if (bits == 0)
            return result;

        size_t blocks = bits / 32;
        uint32_t shiftBits = bits % 32;

        // Dịch block
        result.num.insert(result.num.begin(), blocks, 0);

        if (shiftBits)
        {
            uint64_t carry = 0;
            for (size_t i = 0; i < result.num.size(); ++i)
            {
                uint64_t val = ((uint64_t)result.num[i] << shiftBits) | carry;
                result.num[i] = val & 0xffffffffu;
                carry = val >> 32;
            }
            if (carry)
                result.num.push_back((uint32_t)carry);
        }
        return result;
    }
    BigInt div(const BigInt &other) const
    {
        BigInt quotient(0);
        BigInt remainder(0);

        for (int i = this->num.size() * 32 - 1; i >= 0; --i)
        {
            // Dịch trái remainder 1 bit
            remainder = remainder.shiftLeft(1);
            if ((this->num[i / 32] >> (i % 32)) & 1)
                remainder.num[0] |= 1;

            if (!(remainder < other))
            {
                remainder = remainder - other;
                quotient = quotient.shiftLeft(1);
                quotient.num[0] |= 1;
            }
            else
            {
                quotient = quotient.shiftLeft(1);
            }
        }
        return quotient;
    }

    BigInt mod(const BigInt &other) const
    {
        BigInt remainder(0);

        for (int i = this->num.size() * 32 - 1; i >= 0; --i)
        {
            remainder = remainder.shiftLeft(1);
            if ((this->num[i / 32] >> (i % 32)) & 1)
                remainder.num[0] |= 1;

            if (!(remainder < other))
            {
                remainder = remainder - other;
            }
        }
        return remainder;
    }

    // Compare operator
    int compare(const BigInt &other) const
    {
        if (this->num.size() != other.num.size())
        {
            return this->num.size() > other.num.size() ? 1 : -1;
        }
        size_t bitSize = this->num.size();
        for (size_t i = bitSize; i-- > 0;)
        {
            if (this->num[i] != other.num[i])
            {
                return this->num[i] > other.num[i] ? 1 : -1;
            }
        }

        return 0;
    }

    bool operator<(const BigInt &other) const
    {
        return compare(other) == -1 ? true : false;
    }
    bool operator>= (const BigInt &other) const {
        return (compare(other) == 1 || compare(other) == 0) ? true : false;
    }
    bool operator>(const BigInt &other) const
    {
        return compare(other) == 1 ? true : false;
    }
    bool operator==(const BigInt &other) const
    {
        return compare(other) == 0 ? true : false;
    }
    bool operator!=(const BigInt &other)
    {
        return compare(other) != 0 ? true : false;
    }
    bool isZero()
    {
        return num.size() == 1 && num[0] == 0;
    }
};

vector<BigInt> readFile (string fileName){
    ifstream fi(fileName);

    if (!fi.is_open()){
        cerr << "File is unable to Open!";
        return {};
    }

    string n, k, x;
    fi >> n;
    fi >> k;
    fi >> x;

    return {BigInt::hexToBig(n), BigInt::hexToBig(k), BigInt::hexToBig(x)};

}

BigInt gcdExtended(BigInt a, BigInt b)
{

    // if (b.isZero())
    // {
    //     x = BigInt(1);
    //     y = BigInt(0);
    //     return a;
    // }

    // BigInt x1, y1;
    // BigInt gcd = gcdExtended(b, a.mod(b), x1, y1);

    // x = y1;
    // y = x1 - (a.div(b)) * y1;
    // // y = (x1.mod(b) + b - ((a / b) * y1).mod(b)).mod(b);
    // cout << "invX:"; x.displayNumDec();
    // return gcd;

    BigInt originB = b;
    BigInt x0 = BigInt(0);
    BigInt x1 = BigInt(1);

    if (originB == 1){
        return BigInt(0);
    }

    while (a > BigInt(1)){
        BigInt q = a.div(b);
        BigInt tmpB = b;

        b = a.mod(b);
        a = tmpB;

        tmpB = x0;

        BigInt tmpMul = q * x0;

        if (x1 >= tmpMul){
            x0 = x1 - tmpMul;
        }else{
            x0 = originB - (tmpMul - x1);
        }

        x1 = tmpB;
    }

    return x1.mod(originB);

}


vector<BigInt> initMont (BigInt N, BigInt x, BigInt R){

    // find N' = (-(N^-1) mod R + R) mod R
    // BigInt gcdN = gcdExtended(N, R);
    // if (gcdN != 1){
    //     cerr << "invModN can't be found" << endl;
    //     return {};
    // }
    BigInt invModN = gcdExtended(N, R);
    BigInt invN = (R - invModN);

    // convert x to montgomery number
    BigInt xFormated = (x * R).mod(N);

    return {xFormated, invN};

}

BigInt toMontNum (BigInt x, BigInt N, BigInt R){
    return (x * R).mod(N);
}

BigInt montMul (BigInt a, BigInt b, BigInt N, BigInt invN, BigInt R){

    BigInt T = a * b;
    BigInt m = (T * invN).mod(R);
    BigInt t = (T + m * N).div(R);

    t = t.mod(N);
    return t;

}


BigInt decrypt (BigInt x, BigInt k, BigInt N){

    cout << "N: "; N.displayNum();
    cout << "k: "; k.displayNumBin();
    cout <<"x: "; x.displayNum();

    BigInt R = BigInt(1).shiftLeft(N.bitLenght() + 1); //R > N | N - 11 bits --> R - 12 bits
    BigInt res = toMontNum(BigInt(1), N, R);
    vector<BigInt> initList = initMont(N, x, R);
    BigInt xFormated = initList[0];
    BigInt invN = initList[1]; //N'
    
    cout << "R: "; R.displayNumDec();
    cout << "N': "; invN.displayNumDec();

    for (int block = k.getData().size() - 1; block >= 0; block--) {
        uint32_t bits = k.getData()[block];
        for (int i = 31; i >= 0; i--) { // traverse bits High -> Low
            res = montMul(res, res, N, invN, R); // ^2
            cout << "bit (" << (bits >> i) << ")";
            cout << "square: "; res.displayNum();
            if ((bits >> i) & 1) {
                res = montMul(res, xFormated, N, invN, R); // multiply if bit=1
            }
        }
    }

    return montMul(res, BigInt(1), N, invN, R);

}


int main (int argc, char* argv[]){

    if (argc != 3){
        cerr << "Somethings is Wrong with your input";
        return 1;
    }

    string fileIn = argv[1];
    string fileO = argv[2];

    ofstream fo(fileO);
    
    vector<BigInt> listNum = readFile(fileIn);

    BigInt y = decrypt(listNum[2], listNum[1], listNum[0]);

    fo << y.bigToHex() << endl;

}



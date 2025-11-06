#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <filesystem>

using namespace std;

// Define a custom structure to represent a 512-bit big integer, using 32-bit (4-byte) elements as the base units.
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

    static BigInt hexToBigInt(const string &hexStr)
    {
        BigInt res;
        res.num.clear();
        string s = hexStr;
        string clean;
        for (char c : s)
            if (!isspace(c))
                clean.push_back(c);

        if (clean.size() % 8 != 0)
            clean.append(8 - clean.size() % 8, '0');

        for (size_t i = 0; i < clean.size(); i += 8)
        {
            string part = clean.substr(i, 8);
            uint32_t val = 0;
            for (int j = 0; j < 8; j++)
            {
                char ch = part[j];
                val |= (uint32_t)((ch >= 'A' ? ch - 'A' + 10 : ch - '0')) << (4 * j);
            }
            res.num.push_back(val);
        }
        res.norm();
        return res;
    }

    string bigIntToHex() const
    {
        string s;
        for (uint32_t block : num)
        {
            for (int i = 0; i < 8; i++)
            {
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
        size_t bitSize = max(this->num.size(), other.num.size()); // get the longest bit size

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

// Extend Euclidean GCD
BigInt gcdExtended(BigInt a, BigInt b, BigInt &x, BigInt &y)
{

    if (a.isZero())
    {
        x = 0;
        y = 1;
        return b;
    }

    BigInt x1, y1;
    BigInt gcd = gcdExtended(b.mod(a), a, x1, y1);

    x = y1 - (b / a) * x1;
    y = x1;
    return gcd;
}

// BigInt findPublicKey(BigInt delta)
// {
//     vector<long> listE = {3, 5, 17, 257, 65537};

//     for (int i = 0; i < listE.size(); i++)
//     {
//         BigInt x = 1;
//         BigInt y = 1;

//         if (gcdExtended(listE[i], delta, x, y) == 1)
//         {
//             return listE[i];
//         }
//     }

//     return 0;
// }

BigInt findSecrectKey(BigInt e, BigInt delta)
{

    BigInt a = delta;
    BigInt b = e;

    BigInt x0 = 0;
    BigInt x1 = 1;

    while (!b.isZero())
    {
        BigInt q = a / b;
        BigInt temp = b;
        b = a - q * b;
        a = temp;

        temp = x1;
        x1 = (x0 + delta - (q * x1).mod(delta)).mod(delta);
        x0 = temp;
    }

    if (!(a == 1))
    {
        return 0;
    }

    return x0.mod(delta);
}

BigInt keyGen(BigInt p, BigInt q, BigInt e)
{

    BigInt n = p * q;

    BigInt delta = (--p) * (--q);

    BigInt d = findSecrectKey(e, delta);

    return d;
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        cerr << "Somethings is Wrong with input";
        return 1;
    }

    vector<BigInt> listKey;

    string fileIn = argv[1];
    string fileO = argv[2];

    ifstream fi(fileIn);
    ofstream fo(fileO);

    if (!fi.is_open() || !fo.is_open())
    {
        cerr << "File I/O is not open";
        return 1;
    }

    string pHex, qHex, eHex;
    getline(fi, pHex);
    getline(fi, qHex);
    getline(fi, eHex);

    BigInt q = BigInt::hexToBigInt(qHex);
    BigInt p = BigInt::hexToBigInt(pHex);
    BigInt e = BigInt::hexToBigInt(eHex);

    BigInt d = keyGen(p, q, e);

    if (d.isZero())
    {
        fo << -1 << endl;
    }
    else
    {
        fo << d.bigIntToHex() << endl;
    }
}
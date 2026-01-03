#include <openssl/evp.h>
#include <openssl/pem.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

EVP_PKEY* readPrivateKey(const char* filename) {
    FILE* fi = fopen(filename, "r");
    if (!fi) {
        cerr << "Khong mo duoc private key file\n";
        return nullptr;
    }

    EVP_PKEY* key = PEM_read_PrivateKey(fi, nullptr, nullptr, nullptr);
    fclose(fi);
    return key;
}

bool readMessage(const char* filename, EVP_MD_CTX* ctx) {
    ifstream fi(filename, ios::binary);
    if (!fi) {
        cerr << "Khong mo duoc message file\n";
        return false;
    }

    vector<char> buffer(1024);
    while (fi.good()) {
        fi.read(buffer.data(), buffer.size());
        if (fi.gcount() > 0)
            EVP_DigestSignUpdate(ctx, buffer.data(), fi.gcount());
    }
    return true;
}

bool writeSign(const char* filename,
               const vector<unsigned char>& sig,
               size_t siglen) {
    ofstream fout(filename, ios::binary);
    if (!fout) {
        cerr << "Khong ghi duoc file chu ky\n";
        cerr << filename << "\n";
        return false;
    }
    fout.write((char*)sig.data(), siglen);
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0]
             << " priv.pem mess sign\n";
        return 1;
    }

    const char* privFile = argv[1];
    const char* messFile = argv[2];
    const char* signFile = argv[3];

    EVP_PKEY* privKey = readPrivateKey(privFile);
    if (!privKey) return 1;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return 1;

    if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, privKey) <= 0) {
        cerr << "DigestSignInit failed\n";
        return 1;
    }

    if (!readMessage(messFile, ctx)) return 1;

    size_t siglen = 0;
    EVP_DigestSignFinal(ctx, nullptr, &siglen);

    vector<unsigned char> signature(siglen);
    EVP_DigestSignFinal(ctx, signature.data(), &siglen);

    if (!writeSign(signFile, signature, siglen)) return 1;

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(privKey);

    cout << "Ky thanh cong\n";
    return 0;
}

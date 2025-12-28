#include <openssl/evp.h>
#include <openssl/pem.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

EVP_PKEY* readPublicKey(const char* filename) {
    FILE* fi = fopen(filename, "r");
    if (!fi) {
        cerr << "Khong mo duoc public key file\n";
        return nullptr;
    }

    EVP_PKEY* pubKey = PEM_read_PUBKEY(fi, nullptr, nullptr, nullptr);
    fclose(fi);
    return pubKey;
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
            EVP_DigestVerifyUpdate(ctx, buffer.data(), fi.gcount());
    }
    return true;
}

bool readSignature(const char* filename,
                   vector<unsigned char>& signature) {
    ifstream fi(filename, ios::binary);
    if (!fi) {
        cerr << "Khong mo duoc file chu ky\n";
        return false;
    }

    fi.seekg(0, ios::end);
    size_t siglen = fi.tellg();
    fi.seekg(0, ios::beg);

    signature.resize(siglen);
    fi.read((char*)signature.data(), siglen);

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0]
             << " pub.pem mess sign\n";
        return 1;
    }

    const char* pubFile  = argv[1];
    const char* messFile = argv[2];
    const char* signFile = argv[3];

    EVP_PKEY* pubKey = readPublicKey(pubFile);
    if (!pubKey) return 1;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return 1;

    if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pubKey) <= 0) {
        cerr << "DigestVerifyInit failed\n";
        return 1;
    }

    if (!readMessage(messFile, ctx)) return 1;

    vector<unsigned char> signature;
    if (!readSignature(signFile, signature)) return 1;

    int ret = EVP_DigestVerifyFinal(
        ctx,
        signature.data(),
        signature.size()
    );

    if (ret == 1)
        cout << "Chu ky HOP LE\n";
    else
        cout << "Chu ky KHONG hop le\n";

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pubKey);

    return 0;
}

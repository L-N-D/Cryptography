# HƯỚNG DẪN CHẠY CHƯƠNG TRÌNH RSA SIGN & VERIFY

---

## 1. Chuẩn bị môi trường

Cài đặt các gói cần thiết:

```bash
sudo apt update
sudo apt install g++ openssl libssl-dev
```

Kiểm tra OpenSSL:

```bash
openssl version
```

---

## 2. Chuẩn bị mã nguồn

Thư mục làm việc gồm các file:

```
.
├── sign.cpp
├── verify.cpp
```

> Nội dung `sign.cpp` và `verify.cpp` đúng theo file đã cung cấp trước đó (sử dụng EVP + SHA-256).

---

## 3. Biên dịch chương trình

```bash
g++ sign.cpp -o sign -lcrypto
g++ verify.cpp -o verify -lcrypto
```

Sau bước này sẽ có:

```
./sign
./verify
```

---

## 4. Sinh khóa RSA

### 4.1. Sinh khóa bí mật

```bash
openssl genpkey -algorithm RSA -out priv.pem -pkeyopt rsa_keygen_bits:2048
```

### 4.2. Tách khóa công khai

```bash
openssl pkey -in priv.pem -pubout -out pub.pem
```

Kết quả:

```
priv.pem   (khóa bí mật)
pub.pem    (khóa công khai)
```

---

## 5. Tạo thông điệp cần ký

```bash
echo "Hello RSA Signature" > mess
```

---

## 6. Chạy chương trình ký (SIGN)

Cú pháp:

```bash
./sign priv.pem mess sign
```

Ý nghĩa:

* `priv.pem`: khóa bí mật
* `mess`: thông điệp gốc
* `sign`: file chữ ký sinh ra

Kết quả mong đợi:

```
Ky thanh cong
```

---

## 7. Chạy chương trình xác thực (VERIFY)

Cú pháp:

```bash
./verify pub.pem mess sign
```

Ý nghĩa:

* `pub.pem`: khóa công khai
* `mess`: thông điệp gốc
* `sign`: chữ ký cần kiểm tra

Kết quả:

```
Chu ky HOP LE
```

---

## 8. Kiểm tra nhanh các trường hợp

### 8.1. Thay đổi nội dung thông điệp

```bash
echo "Fake message" > mess
./verify pub.pem mess sign
```

Kết quả:

```
Chu ky KHONG hop le
```

### 8.2. Thay đổi chữ ký

```bash
echo "abc" > sign
./verify pub.pem mess sign
```

Kết quả:

```
Chu ky KHONG hop le
```

---

## 9. Kết luận

* Quy trình: **Sinh khóa → Ký → Xác thực**
* Thuật toán: **RSA + SHA-256 (EVP API)**
* Có thể dùng để demo, báo cáo hoặc kiểm tra chữ ký điện tử

---

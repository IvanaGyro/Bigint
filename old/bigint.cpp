#include "bigint.h"
#include <stdlib.h>
#include <iostream>

#include <random>

namespace Old {

Bigint::Bigint(int bits)
{
    this->bits = bits;
    if(!this->bits) this->bits = 1;
    if(this->bits&31) bytes4_num = (this->bits>>5)+1;
    else bytes4_num = this->bits>>5;
    num = (unsigned int*)malloc(sizeof(int)*bytes4_num);
}

Bigint::Bigint()
{
    bits = 32;
    bytes4_num = 1;
    num = (unsigned int*)malloc(sizeof(int)*bytes4_num);
}

Bigint::Bigint(const Bigint& B){
    this->bits = B.bits;
    this->bytes4_num = B.bytes4_num;
    num = (unsigned int*)malloc(sizeof(unsigned int)*bytes4_num);
    *this = B;
}

Bigint::~Bigint(){
    //std::cout << "free" << this << std::endl;
    free(num);
}


unsigned int* Bigint::getnum()const{
    return num;
}

unsigned int Bigint::getbytes4_num()const{
    return bytes4_num;
}

void Bigint::setbyte4_num(unsigned int bytes){
    free(num);
    bytes4_num = bytes;
    num = (unsigned int*)malloc(sizeof(unsigned int)*bytes4_num);
}

void Bigint::setbits(unsigned int bits){
    this->bits = bits;
    if(!this->bits){
        this->bits = 1;
        this->setbyte4_num(1);
    }
    if(this->bits&31) setbyte4_num((this->bits>>5)+1);
    else setbyte4_num(this->bits>>5);
}

void Bigint::operator=(const Bigint& B){
    if(bytes4_num < B.getbytes4_num()){
        for(unsigned int i = 0; i<bytes4_num; i++){
            *(num+i) = *(B.getnum()+i);
        }
    }
    else{
        for(unsigned int i = 0; i<B.getbytes4_num(); i++){
            *(num+i) = *(B.getnum()+i);
        }
        for(unsigned int i = B.getbytes4_num(); i < bytes4_num; i++){
            *(num+i) = 0;
        }
    }
}

void Bigint::operator=(const unsigned int& I){
    for(unsigned int i = 0; i < bytes4_num; i++) *(num+i) = 0;
    *num = I;
}

bool Bigint::operator>(const unsigned int& I)const{
    for(unsigned int i = this->bytes4_num; i>1; i--){
        if(*(this->num+i-1)) return true;
    }
    if(*(this->num)>I) return true;
    else return false;
}

bool Bigint::operator>(const Bigint& B)const{
    unsigned int i;
    if(B.bytes4_num > this->bytes4_num){
        for(i = B.bytes4_num; i>this->bytes4_num; i--){
            if(*(B.num+i-1)) return false;
        }
    }
    else if(this->bytes4_num > B.bytes4_num){
        for(i = this->bytes4_num; i>B.bytes4_num; i--){
            if(*(this->num+i-1)) return true;
        }
    }
    for(i = this->bytes4_num>B.bytes4_num?B.bytes4_num:this->bytes4_num; i>1; i--){
        if(*(this->num+i-1)>*(B.num+i-1)) return true;
        else if(*(this->num+i-1)<*(B.num+i-1)) return false;
    }
    if(*(this->num)>*(B.num)) return true;
    else return false;
}

bool Bigint::operator==(const unsigned int& I)const{
    for(unsigned int i = this->bytes4_num; i>1; i--){
        if(*(this->num+i-1)) return false;
    }
    if(*(this->num)==I) return true;
    else return false;
}

bool Bigint::operator==(const Bigint& B)const{
    unsigned int i;
    if(B.bytes4_num > this->bytes4_num){
        for(i = B.bytes4_num; i>this->bytes4_num; i--){
            if(*(B.num+i-1)) return false;
        }
    }
    else if(this->bytes4_num > B.bytes4_num){
        for(i = this->bytes4_num; i>B.bytes4_num; i--){
            if(*(this->num+i-1)) return false;
        }
    }
    for(i = this->bytes4_num>B.bytes4_num?B.bytes4_num:this->bytes4_num; i>1; i--){
        if(*(this->num+i-1)!=*(B.num+i-1)) return false;
    }
    if(*(this->num)==*(B.num)) return true;
    else return false;
}

bool Bigint::operator<(const unsigned int& I)const{
    for(unsigned int i = this->bytes4_num; i>1; i--){
        if(*(this->num+i-1)) return false;
    }
    if(*(this->num)<I) return true;
    else return false;
}

bool Bigint::operator<(const Bigint& B)const{
    unsigned int i;
    if(B.bytes4_num > this->bytes4_num){
        for(i = B.bytes4_num; i>this->bytes4_num; i--){
            if(*(B.num+i-1)) return true;
        }
    }
    else if(this->bytes4_num > B.bytes4_num){
        for(i = this->bytes4_num; i>B.bytes4_num; i--){
            if(*(this->num+i-1)) return false;
        }
    }
    for(i = this->bytes4_num>B.bytes4_num?B.bytes4_num:this->bytes4_num; i>1; i--){
        if(*(this->num+i-1)<*(B.num+i-1)) return true;
        else if(*(this->num+i-1)>*(B.num+i-1)) return false;
    }
    if(*(this->num)<*(B.num)) return true;
    else return false;
}

bool Bigint::operator>=(const unsigned int& I)const{
    return !(*this < I);
}

bool Bigint::operator>=(const Bigint& B)const{
    return !(*this < B);
}

bool Bigint::operator<=(const unsigned int& I)const{
    return !(*this > I);
}

bool Bigint::operator<=(const Bigint& B)const{
    return !(*this > B);
}

bool Bigint::operator!=(const unsigned int& I)const{
    return !(*this == I);
}

bool Bigint::operator!=(const Bigint& B)const{
    return !(*this == B);
}

Bigint Bigint::operator &(const unsigned int& I)const{
    Bigint tmp(this->bytes4_num*32);
    tmp = 0;
    *tmp.num = *(this->num) & I;
    return Bigint(tmp);
}

Bigint Bigint::operator &(const Bigint& B)const{
    unsigned int bigger_bytes, smaller_bytes, i;
    bigger_bytes = this->bytes4_num>B.bytes4_num?this->bytes4_num:B.bytes4_num;
    smaller_bytes = this->bytes4_num<B.bytes4_num?this->bytes4_num:B.bytes4_num;
    Bigint tmp(bigger_bytes*32);
    tmp = *this;
    for(i = 0; i<smaller_bytes; i++){
        *(tmp.num+i) &= *(B.num+i);
    }
    for(i = smaller_bytes; i<bigger_bytes; i++){
        *(tmp.num+i) = 0;
    }
    return Bigint(tmp);
}

void Bigint::operator &=(const unsigned int& I){
    *this = *this & I;
}

void Bigint::operator &=(const Bigint& B){
    *this = *this & B;
}

Bigint Bigint::operator ^(const unsigned int& I){
    Bigint tmp(this->bytes4_num*32);
    tmp = 0;
    *(tmp.num) = *(this->num) ^ I;
    return Bigint(tmp);
}

Bigint Bigint::operator ^(const Bigint& B){
    unsigned int bigger_bytes, i;
    bigger_bytes = this->bytes4_num>B.bytes4_num?this->bytes4_num:B.bytes4_num;
    Bigint tmp(bigger_bytes*32);
    tmp = *this;
    for(i = 0; i<B.bytes4_num; i++){
        *(tmp.num+i) ^= *(B.num+i);
    }
    return Bigint(tmp);
}

void Bigint::operator ^=(const unsigned int& I){
    *this = *this ^ I;
}

void Bigint::operator ^=(const Bigint& B){
    *this = *this ^ B;
}

Bigint Bigint::operator<<(const unsigned int& I){
    Bigint tmp(bytes4_num*32);
    tmp = *this;
    unsigned int carry, carry_pre = 0, I_num;
    unsigned int mask;

    if(I >= 32){
        I_num = I >> 5;
        for(unsigned int i = tmp.bytes4_num - 1; i >= I_num; i--){
            *(tmp.num+i) = *(tmp.num+i-I_num);
        }
        for(unsigned int i = 0; i < I_num; i++){
            *(tmp.num+i) = 0;
        }
    }
    if(I){
        I_num = I & 31;
        mask = ((1<<I_num)-1)<<(32-I_num);
        for(unsigned int i = 0; i < tmp.bytes4_num; i++){
            carry = mask & *(tmp.num+i);
            *(tmp.num+i) <<= I_num;
            *(tmp.num+i) ^= carry_pre;
            carry_pre = carry >> (32-I_num);
        }
    }
    return Bigint(tmp);
}

void Bigint::operator <<=(const unsigned int& I){
    *this = *this << I;
}

Bigint Bigint::operator>>(const unsigned int& I){
    Bigint tmp(bytes4_num*32);
    tmp = *this;
    unsigned int carry, carry_pre = 0, I_num = I;
    unsigned int mask;
    while(I_num){
        if(I_num < 32){
            mask = (1<<I_num)-1;
            for(unsigned int i = tmp.bytes4_num; i > 0 ; i--){
                carry = mask & *(tmp.num+i-1);
                *(tmp.num+i-1) >>= I_num;
                *(tmp.num+i-1) ^= carry_pre;
                carry_pre = carry << (32-I_num);
            }
            I_num = 0;
        }
        else{
            for(unsigned int i = 0; i < tmp.bytes4_num-1; i++){
                *(tmp.num+i) = *(tmp.num+i+1);
            }
            *(tmp.num+tmp.bytes4_num-1) = 0;
            I_num -= 32;
        }
    }
    return Bigint(tmp);
}

void Bigint::operator >>=(const unsigned int& I){
    *this = *this >> I;
}

Bigint Bigint::operator +(const unsigned int& I){
    Bigint tmp(this->bits);
    tmp = *this;
    unsigned int tmp_B1, tmp_B2, carry = 0, i = 1;
    tmp_B1 = *(tmp.num)&0x80000000;
    tmp_B2 = I&0x80000000;
    if(tmp_B1 && tmp_B2){
        carry = 1;
    }
    else if(tmp_B1 || tmp_B2){
        tmp_B1 = *(tmp.num)&0x7FFFFFFF;
        tmp_B2 = I&0x7FFFFFFF;
        if((tmp_B1+tmp_B2)&0x80000000) carry = 1;
        else carry = 0;
    }
    else carry = 0;
    *(tmp.num) = I + *(tmp.num);
    if(tmp.bytes4_num>1){
        while(!(~*(tmp.num+i))){
            *(tmp.num+i) += carry;
            i++;
        }
        if(i<tmp.bytes4_num) *(tmp.num+i) += carry;
    }
    return Bigint(tmp);
}

Bigint Bigint::operator +(const Bigint& B){
    Bigint tmp(bytes4_num > B.getbytes4_num()?bytes4_num*32:B.getbytes4_num()*32);
    unsigned int tmp_B1, tmp_B2, carry = 0, carry_pre = 0, j = 0;
    tmp = *this;
    for(unsigned int i = 0; i<(bytes4_num<B.getbytes4_num()?bytes4_num:B.getbytes4_num()); i++){
        tmp_B1 = *(tmp.num+i)&0x80000000;
        tmp_B2 = *(B.num+i)&0x80000000;
        if(tmp_B1 && tmp_B2){
            carry = 1;
        }
        else if(tmp_B1 || tmp_B2){
            tmp_B1 = *(tmp.num+i)&0x7FFFFFFF;
            tmp_B2 = *(B.num+i)&0x7FFFFFFF;
            if((tmp_B1+tmp_B2)&0x80000000) carry = 1;
            else carry = 0;
        }
        else carry = 0;
        *(tmp.num+i) = *(B.num+i) + *(tmp.num+i);
        j = 0;
        while(i+j<(bytes4_num<B.getbytes4_num()?bytes4_num:B.getbytes4_num())&&!(~*(tmp.num+i+j))){
            *(tmp.num+i+j) = carry_pre +*(tmp.num+i+j);
            j++;
        }
        if(i+j<(bytes4_num<B.getbytes4_num()?bytes4_num:B.getbytes4_num())){
            *(tmp.num+i+j) = carry_pre +*(tmp.num+i+j);
        }
        carry_pre = carry;
    }
    return Bigint(tmp);
}

void Bigint::operator +=(const unsigned int& I){
    *this = *this + I;
}

void Bigint::operator +=(const Bigint& B){
    *this = *this + B;
}

Bigint Bigint::operator -(const unsigned int& I)const{
    Bigint tmp(this->bytes4_num*32);
    unsigned int i = 1;
    tmp = *this;
    if(*(tmp.num) >= I){
        *(tmp.num) -= I;
        return Bigint(tmp);
    }
    if(tmp < I){
        tmp = 0;
        return Bigint(tmp);
    }
    if(*(tmp.num)<I){
        *(tmp.num) += ((~I)+1);
        while(!*(tmp.num+i)) i++;
        *(tmp.num+i) -= 1;
    }
    return Bigint(tmp);
}

Bigint Bigint::operator-(const Bigint& B)const{
    Bigint tmp(this->bytes4_num*32), subed(B.bytes4_num*32), sub(B.bytes4_num*32);
    unsigned int i;
    sub = B;
    if(sub > *this){
        tmp = 0;
        return Bigint(tmp);
    }
    tmp = *this;
    if(sub.bytes4_num > tmp.bytes4_num) subed = tmp;
    else{
        for(i = 0; i<sub.bytes4_num; i++){
            *(subed.num+i) = *(tmp.num+i);
        }
    }
    if(subed < sub){
        for(i = 0; i<sub.bytes4_num; i++){
            *(sub.num+i) = ~*(sub.num+i);
        }
        sub += 1;
        subed += sub;
        for(i = 0; i<sub.bytes4_num; i++){
            *(tmp.num+i) = *(subed.num+i);
        }
        i = sub.bytes4_num;
        while(!*(tmp.num+i)) i++;
        *(tmp.num+i) -= 1;
    }
    else{
        for(i = 0; i<sub.bytes4_num; i++){
            *(sub.num+i) = ~*(sub.num+i);
        }
        sub += 1;
        subed += sub;
        for(i = 0; i<(sub.bytes4_num<tmp.bytes4_num?sub.bytes4_num:tmp.bytes4_num); i++){
            *(tmp.num+i) = *(subed.num+i);
        }
    }
    return Bigint(tmp);
}

void Bigint::operator -=(const unsigned int& I){
    *this = *this - I;
}

void Bigint::operator -=(const Bigint& B){
    *this = *this - B;
}

Bigint Bigint::operator*(const unsigned int& I)const{
    Bigint tmp((this->bytes4_num+1)*32), this_cpy((this->bytes4_num+1)*32);
    unsigned int check = 1;
    this_cpy = *this;
    tmp = 0;
    for(unsigned int i = 0; i<32; i++){
        if(check & I) tmp+=(this_cpy<<i);
        check <<= 1;
    }
    return Bigint(tmp);
}

Bigint Bigint::operator*(const Bigint& B)const{
    Bigint tmp((this->bytes4_num+B.bytes4_num)*32), this_cpy((this->bytes4_num+B.bytes4_num)*32);
    unsigned int check = 1;
    this_cpy = *this;
    tmp = 0;
    for(unsigned int i = 0; i<B.bytes4_num; i++){
        for(unsigned int j = 0; j<32 ;j++){
            if(check & *(B.num+i)){
                tmp+=(this_cpy<<(i*32+j));
            }
            check <<= 1;
        }
        check = 1;
    }
    return Bigint(tmp);
}

void Bigint::operator *=(const unsigned int& I){
    *this = *this * I;
}

void Bigint::operator *=(const Bigint& B){
    *this = *this * B;
}

Bigint Bigint::operator/(const unsigned int& I)const{
    Bigint dived(this->bytes4_num*32), divor(this->bytes4_num*32), quo(this->bytes4_num*32);
    unsigned int i, check = 0x80000000, shift_divor = 0;
    dived = *this;
    divor = I;
    quo = 0;
    if(dived < divor) return Bigint(quo);
    while(!(check&*(divor.num+divor.bytes4_num-1))){
        divor <<= 1;
        shift_divor++;
    }
    for(i = 0; i < shift_divor; i++){
        if(dived>=divor){
            dived -= divor;
            *(quo.num) |= 1;
        }
        quo <<= 1;
        divor >>= 1;
    }

    if(dived>=divor) *(quo.num) |= 1;
    return Bigint(quo);
}

Bigint Bigint::operator/(const Bigint& B)const{
    Bigint dived(this->bytes4_num*32), divor(this->bytes4_num*32), quo(this->bytes4_num*32);
    unsigned int i, check = 0x80000000, shift_divor = 0;
    dived = *this;
    quo = 0;
    if(B > dived){
        return Bigint(quo);
    }
    divor = B;
    while(!(check&*(divor.num+dived.bytes4_num-1))){
        divor <<= 1;
        shift_divor++;
    }
    for(i = 0; i < shift_divor; i++){
        if(dived>=divor){
            dived -= divor;
            *(quo.num) |= 1;
        }
        quo <<= 1;
        divor >>= 1;
    }
    if(dived>=divor) *(quo.num) |= 1;
    return Bigint(quo);
}

void Bigint::operator /=(const unsigned int& I){
    *this = *this / I;
}

void Bigint::operator /=(const Bigint& B){
    *this = *this / B;
}

unsigned int Bigint::operator%(const unsigned int& I){
    Bigint dived(this->bytes4_num*32), divor(this->bytes4_num*32);
    unsigned int i, check = 0x80000000, shift_divor = 0;
    dived = *this;
    divor = I;
    if(dived < divor) return *(dived.num);
    while(!(check&*(divor.num+divor.bytes4_num-1))){
        divor <<= 1;
        shift_divor++;
    }
    for(i = 0; i < shift_divor; i++){
        if(dived>=divor) dived -= divor;
        divor >>= 1;
    }

    if(dived>=divor) dived -= divor;
    return *dived.num;
}

Bigint Bigint::operator%(const Bigint& B){
    Bigint dived(this->bytes4_num*32), divor(B.bytes4_num*32), divor2(this->bytes4_num*32);
    unsigned int i, check = 0x80000000, shift_divor = 0;
    dived = *this;
    divor = B;
    if(divor > dived){
        return Bigint(dived);
    }
    divor2 = B;
    while(!(check&*(divor2.num+dived.bytes4_num-1))){
        divor2 <<= 1;
        shift_divor++;
    }
    for(i = 0; i < shift_divor; i++){
        if(dived>=divor2) dived -= divor2;
        divor2 >>= 1;
    }
    if(dived>=divor2) dived -= divor2;
    divor = dived;
    return Bigint(divor);
}

void Bigint::operator %=(const unsigned int& I){
    *this = *this % I;
}

void Bigint::operator %=(const Bigint& B){
    *this = *this % B;
}

void Bigint::random1__1(){
    this->random();
    *(this->num+this->bytes4_num-1) |= 0x80000000;
    *(this->num) |= 1;
}

void Bigint::random(){
    unsigned int shift_times;
    shift_times = rand()>>11;
    shift_times += (this->bytes4_num*32/15+1);
    for(unsigned int i = 0; i<shift_times; i++){
        *this = *this<<15;
        *(this->num) ^= rand();
    }
}

void Bigint::print_bin()const{
    unsigned int check_bit = 1<<31;
    int i, j;
    for(i = bytes4_num-1; i>=0; i--){
        for(j = 0; j<32; j++){
            if(check_bit&*(num+i)){
                std::cout << '1';

            }
            else{
                std::cout << '0';

            }
            check_bit >>= 1;
        }
        check_bit = 1<<31;
        std::cout << " " << i << " ";
    }
    std::cout << std::endl;
}

std::string Bigint::toString_dec()const{
    Bigint tmp(*this);
    std::string dec_str = "";
    char c;
    while(tmp >= 10){
        c = tmp%10+48;
        dec_str = c + dec_str;
        tmp /= 10;
    }
    c = tmp%10+48;
    dec_str = c + dec_str;
    return dec_str;
}

std::string Bigint::toString_0dec()const{
    Bigint tmp(*this);
    std::string dec_str = this->toString_dec();
    while(dec_str.size() < (int)(this->bytes4_num*32*_log2)+1){
        dec_str = '0' + dec_str;
    }
    return dec_str;
}

std::string Bigint::toString_bin()const{
    std::string bin_str = "";
    unsigned int check_bit = 1<<31;
    int i, j;
    for(i = bytes4_num-1; i>=0; i--){
        for(j = 0; j<32; j++){
            if(check_bit&*(num+i)){
                bin_str += '1';

            }
            else{
                bin_str += '0';

            }
            check_bit >>= 1;
        }
        check_bit = 1<<31;
    }
    return bin_str;
}

}
//https://www.youtube.com/watch?v=9QsKUZatKj0&feature=youtu.be
//https://www.youtube.com/watch?v=V6M7Zo8M8cs



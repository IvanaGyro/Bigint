#ifndef BIGINT_H
#define BIGINT_H

#include <string.h>
#include <iostream>

#define _log2 0.30102
namespace Old {

class Bigint
{
public:
    Bigint();
    Bigint(int bits);
    Bigint(const Bigint&);
    ~Bigint();
    unsigned int* getnum() const;
    unsigned int getbytes4_num() const;
    void setbyte4_num(unsigned int);
    void setbits(unsigned int);
    void operator=(const Bigint&);
    void operator=(const unsigned int&);

    bool operator>(const unsigned int&)const;
    bool operator>(const Bigint&) const;
    bool operator==(const unsigned int&)const;
    bool operator==(const Bigint&) const;
    bool operator<(const unsigned int&)const;
    bool operator<(const Bigint&)const;
    bool operator>=(const unsigned int&)const;
    bool operator>=(const Bigint&)const;
    bool operator<=(const unsigned int&)const;
    bool operator<=(const Bigint&)const;
    bool operator!=(const unsigned int&)const;
    bool operator!=(const Bigint&)const;

    Bigint operator &(const unsigned int&)const;
    Bigint operator &(const Bigint&)const;
    void operator &=(const unsigned int&);
    void operator &=(const Bigint&);

    Bigint operator ^(const unsigned int&);
    Bigint operator ^(const Bigint&);
    void operator ^=(const unsigned int&);
    void operator ^=(const Bigint&);

    Bigint operator<<(const unsigned int&);
    void operator<<=(const unsigned int&);
    Bigint operator>>(const unsigned int&);
    void operator>>=(const unsigned int&);

    Bigint operator+(const unsigned int&);
    Bigint operator+(const Bigint&);
    void operator+=(const unsigned int&);
    void operator+=(const Bigint&);

    Bigint operator-(const unsigned int&)const;
    Bigint operator-(const Bigint&)const;
    void operator-=(const unsigned int&);
    void operator-=(const Bigint&);

    Bigint operator*(const unsigned int&)const;
    Bigint operator*(const Bigint&)const;
    void operator*=(const unsigned int&);
    void operator*=(const Bigint&);

    Bigint operator/(const unsigned int&)const;
    Bigint operator/(const Bigint&)const;
    void operator/=(const unsigned int&);
    void operator/=(const Bigint&);

    unsigned int  operator%(const unsigned int&);
    Bigint operator%(const Bigint&);
    void operator%=(const unsigned int&);
    void operator%=(const Bigint&);

    void random1__1();
    void random();

    void print_bin()const;
    std::string toString_dec()const;
    std::string toString_0dec()const;
    std::string toString_bin()const;


private:
    unsigned int bits;
    unsigned int* num;
    unsigned int bytes4_num;
};

}


#endif // BIGINT_H

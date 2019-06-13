#include <cstdio>
#include <utility>

class C {
  public:
    C() {}
    C(const C&) { puts("copy ctor"); }
    C(C&&) { puts("move ctor"); }
};

C make_C(bool cond) { C c; C d; return cond ? c : d; }

int main()
{
  C c1;
  C c2(make_C(false));
  C c3(std::move(c1));
  C &c4 = c3;
}

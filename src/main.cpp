#include <iostream>

using namespace std;

struct B
{
  B()
  {
  }
};

struct A
{
  A()
  {
    cout << "Default constructor" << endl;
  }

  A(const A &a)
  {
    cout << "Copy constructor" << endl;
  }

  A &operator=(const A &a)
  {
    cout << "Assign operator" << endl;
    return *this;
  }

  A &operator=(const A &&a)
  {
    cout << "Move assign operator" << endl;
    return *this;
  }

  A(const A &&a)
  {
    cout << "Move constructor" << endl;
  }
};

int main(int argc, char *argv[])
{
  A a;
  A a2 = a;
  A a3(a);
  A a4 = std::move(a);
  A a5(std::move(a));
  a2 = a3;
  a2 = std::move(a3);
  std::swap(a2, a3);
  exit(1);
}

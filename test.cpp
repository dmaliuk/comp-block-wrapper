#include <utility>
#include <tuple>
#include <vector>
#include <iostream>

using namespace std;

template <typename Child>
struct Base
{
  void interface()
  {
    Instance().implementation();
    //static_cast<Child*>(this)->implementation();
  }

  Child& Instance()
  {
    return static_cast<Child&>(*this);
  }
};

struct Derived : Base<Derived>
{
    void implementation()
    {
        cerr << "Derived implementation\n";
    }
};

int main()
{
   Derived d;
   d.interface();  // Prints "Derived implementation"
}

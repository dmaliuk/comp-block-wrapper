#include <utility>
#include <tuple>
//#include <string>
//#include <iostream>

template <class F, class Tuple, size_t... I>
inline void  call_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
{
  std::forward<F>(f)(std::get<I>(t)...);
}

template <class F, class Tuple>
inline void call(F&& f, Tuple&& t)
{
  call_impl(std::forward<F>(f), std::forward<Tuple>(t), std::make_index_sequence<std::tuple_size<Tuple>::value>{});
}

double result = 0.;

struct Foo {
  int i;
  int ii[10];
  double d;
  
  Foo(int i_ = 0, double d_ = 0.):i(i_), d(d_)
  {
    for (int k=0; k < 10; ++k)
      ii[k] = k * i;
  }
};

void print(double d, int s, Foo const& foo)
{
  //std::cout << "d " << d << " s " << s << std::endl;
  result += d + static_cast<double>(s);
  result += foo.d;
}


int main()
{
  double d = 3.14;
  //std::string s{"doom"};
  int i = 12;
  Foo foo{2,2.111};
  call(print, std::tie(d, i, foo));
}
      

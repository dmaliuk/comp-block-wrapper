#pragma once
#include <type_traits>

template <class...>
using void_t = void;

namespace detail {
template <class Default, class AlwaysVoid,
          template<class...> class Op, class... Args>
struct detector {
  using value_t = std::false_type;
  using type = Default;
};
 
template <class Default, template<class...> class Op, class... Args>
struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
  // Note that std::void_t is a C++17 feature
  using value_t = std::true_type;
  using type = Op<Args...>;
};
 
} // namespace detail
 
template <template<class...> class Op, class... Args>
using is_detected = typename detail::detector<void, void, Op, Args...>::value_t;
 
template <template<class...> class Op, class... Args>
using detected_t = typename detail::detector<void, void, Op, Args...>::type;
 
template <class Default, template<class...> class Op, class... Args>
using detected_or = detail::detector<Default, void, Op, Args...>;


// user data
class QuoteData {
 public:
  QuoteData(int n):quotes(n){};

  decltype(auto) get(int i) const {
    return quotes[i];
  }

  decltype(auto) get(int i) {
    return quotes[i];
  }

  auto size() const { return quotes.size();}

 private:
  std::vector<double> quotes;
};

struct Trade
{
  double price;
  int size;
};

struct Bid
{
  double price;
  int size;
};

struct Ask
{
  double price;
  int size;
};

struct Context {};

void Assert(bool cond)
{
  if (!cond)
    throw std::exception{};
}

template <class... Args> struct type_list{};

#include <utility>
#include <tuple>
#include <vector>
#include <iostream>
#include <exception>
#include <unordered_map>
#include <string>
#include "meta.h"

template <class Indicator, class... Args>
using step_impl_test = decltype(std::declval<Indicator&>().StepImpl(std::declval<Args&>()...), std::true_type{});

template <class Indicator, class... Args>
using update_impl_test = decltype(std::declval<Indicator&>().UpdateImpl(std::declval<Args&>()...), std::true_type{});

template <class Indicator, class... Args>
using post_update_impl_test = decltype(std::declval<Indicator&>().PostUpdateImpl(std::declval<Args&>()...), std::true_type{});

template <class Indicator>
class IndicatorWithInputsAndContext
{
public:
  template <class Context, class Inputs>
  IndicatorWithInputsAndContext(Context const& ctx, Inputs const& inputNames)
  {
    Instance().ForEachInput([&inputNames, this](auto idx, auto name, auto& inputPtr) {
        std::cout << "Initializing input " << name << std::endl;
        auto it = inputNames.find(name);
        std::cout << "found pointer " << it->second << std::endl;
        Assert(it != inputNames.end());
        inputPtr = static_cast<std::decay_t<decltype(inputPtr)>>(it->second);
      });

    // validate inputs
    Instance().ForEachInput([](auto idx, auto name, auto& inputPtr) {
        Assert(inputPtr);
      });
  }

  Indicator& Instance()
  {
    return static_cast<Indicator&>(*this);
  }

  template <class Event>
  constexpr static auto CanStepOn()
  {
    return CanStepOnImpl<Event>(Indicator::InputTypeList());
  }

  template <class Event, class... Args>
  constexpr static auto CanStepOnImpl(type_list<Args...>)
  {
    return is_detected<step_impl_test, Indicator, Event, Context*, Args...>{};
  }

  constexpr static auto CanUpdate()
  {
    return CanUpdateImpl(Indicator::InputTypeList());
  }

  template <class... Args>
  constexpr static auto CanUpdateImpl(type_list<Args...>)
  {
    return is_detected<update_impl_test, Indicator, Context*, Args...>{};
  }

  constexpr static auto CanPostUpdate()
  {
    return CanPostUpdateImpl(Indicator::InputTypeList());
  }

  template <class... Args>
  constexpr static auto CanPostUpdateImpl(type_list<Args...>)
  {
    return is_detected<post_update_impl_test, Indicator, Context*, Args...>{};
  }

  // template <template <class...> class Tester, class... Args>
  // constexpr static auto CanCall()
  // {
  //   return CanCallImpl<Tester, Args...>(Indicator::InputTypeList());
  // }

  // template <template <class...> class Tester, class... Args1, class... Args2>
  // constexpr static auto CanCallImpl(type_list<Args2...>)
  // {
  //   return is_detected<Tester, Indicator, Args1..., Context*, Args2...>{};
  // }

  template <class Context>
  auto Step(Trade const & e, Context const * c) -> std::enable_if_t<IndicatorWithInputsAndContext::CanStepOn<Trade>()>
  {
    StepHelper(e, c, std::make_index_sequence<Indicator::NumInputs()>{});
  }

  template <class Context>
  auto Step(Bid const & e, Context const * c) -> std::enable_if_t<IndicatorWithInputsAndContext::CanStepOn<Bid>()>
  {
    StepHelper(e, c, std::make_index_sequence<Indicator::NumInputs()>{});
  }

  template <class Context>
  auto Step(Ask const & e, Context const * c) -> std::enable_if_t<IndicatorWithInputsAndContext::CanStepOn<Ask>()>
  {
    StepHelper(e, c, std::make_index_sequence<Indicator::NumInputs()>{});
  }

  template <class Event, class Context, size_t... I>
  void StepHelper(Event const & e, Context const * c, std::index_sequence<I...>)
  {
    auto ptrs = Instance().AsTuple();
    Instance().StepImpl(e, c, *std::get<I>(ptrs)...);
  }

  template <class Context>
  void Update(Context const * c)
  {
    UpdateHelper(c, std::make_index_sequence<Indicator::NumInputs()>{});
  }

  template <class Context, size_t... I>
  void UpdateHelper(Context const * c, std::index_sequence<I...>)
  {
    auto ptrs = Instance().AsTuple();
    Instance().UpdateImpl(c, *std::get<I>(ptrs)...);
  }

  template <class Context>
  void PostUpdate(Context const * c)
  {
    PostUpdateHelper(c, std::make_index_sequence<Indicator::NumInputs()>{});
  }

  template <class Context, size_t... I>
  void PostUpdateHelper(Context const * c, std::index_sequence<I...>)
  {
    auto ptrs = Instance().AsTuple();
    Instance().PostUpdateImpl(c, *std::get<I>(ptrs)...);
  }
  
};

class MyIndicator : public IndicatorWithInputsAndContext<MyIndicator>
{
  // hidden by macros
 private:
  int* a;
  double* b;
  QuoteData* quoteData;

 public:
  constexpr static auto InputTypeList() {
    return type_list<int, double, QuoteData>{};
  }

  inline auto AsTuple() const {
    return std::tie(a, b, quoteData);
  }
    
  template <class F>
  void ForEachInput(F&& f)
  {
    f(0, "a", a);
    f(1, "b", b);
    f(2, "quoteData", quoteData);
  }
  
  constexpr static size_t NumInputs()
  {
    return 3;
  }

 public:
  double z = 0.9;
  double tradeBidState = 0.;
  double tradeAskState = 0.;
  double tradeBid = 0.;
  double tradeAsk = 0.;
  double output = 0.;

  
  using Base = IndicatorWithInputsAndContext<MyIndicator>;
  using Base::Step;

  template <class Context, class Inputs>
  MyIndicator(Context const& ctx, Inputs const& inputNames): Base(ctx, inputNames)
  {
  }

  template <class Context>
  void StepImpl(Trade const& t, Context const* ctx, int a, double b, QuoteData const& qd)
  {
    auto const& quotes = qd.get(t.exch);
    auto const& ask = quotes.ask.price;
    auto const& bid = quotes.bid.price;
    
    if (ask > 0. && t.price >= quotes.ask.price)
      tradeAsk += t.size;
    else if (bid >= 0. && t.price <= quotes.bid.price)
      tradeBid += t.size;
  }

  template <class Context>
  inline void StepImpl(Ask const& a, Context const* ctx, int, double, QuoteData& qd)
  {
    auto& ask = qd.get(a.exch).ask;
    ask.price = a.price;
    ask.size = a.size;
  }

  template <class Context>
  inline void StepImpl(Bid const& a, Context const* ctx, int, double, QuoteData& qd)
  {
    auto& bid = qd.get(a.exch).bid;
    bid.price = a.price;
    bid.size = a.size;
  }

  template <class Context>
  inline void UpdateImpl(Context const* ctx, int, double, QuoteData const& qd)
  {
    tradeAskState = z * tradeAskState + (1-z) * tradeAsk;
    tradeBidState = z * tradeBidState + (1-z) * tradeBid;
    auto tot = tradeAskState + tradeBidState;
    output = tot != 0. ? (tradeAskState - tradeBidState)/tot : 0.;
  }

  template <class Context>
  void PostUpdateImpl(Context const*, ...)
  {
    tradeAsk = tradeBid = 0.;
  }
};

static_assert(MyIndicator::CanStepOn<Trade>(), "");
static_assert(MyIndicator::CanStepOn<Ask>(), "");
static_assert(MyIndicator::CanStepOn<Bid>(), "");
static_assert(MyIndicator::CanUpdate(), "");
static_assert(MyIndicator::CanPostUpdate(), "");

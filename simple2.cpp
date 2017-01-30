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

  template <class Context, size_t... I>
  void StepHelper(Trade const & e, Context const * c, std::index_sequence<I...>)
  {
    auto ptrs = Instance().AsTuple();
    Instance().StepImpl(e, c, *std::get<I>(ptrs)...);
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
  using Base = IndicatorWithInputsAndContext<MyIndicator>;
  using Base::Step;

  template <class Context, class Inputs>
  MyIndicator(Context const& ctx, Inputs const& inputNames): Base(ctx, inputNames)
  {
    std::cout << "a: " << a << " b: " << b << std::endl;
  }

  template <class Context>
  void StepImpl(Trade const& t, Context const* ctx, int a, double b, QuoteData const& qd)
  {
    std::cout << "a: " << a << " b: " << b << std::endl;
    std::cout << "contect of quote data: ";
    for (int i = 0; i < qd.size(); ++i)
    {
      std::cout << i << ":" << qd.get(i) << " ";
    }
    std::cout << std::endl;
  }

  template <class Context>
  inline void StepImpl(Ask const& t, Context const* ctx, ...)
  {
    std::cout << "Stepping on Ask" << std::endl;
  }

  template <class Context>
  inline void UpdateImpl(Context const* ctx, ...)
  {
    std::cout << "Updating..." << std::endl;
  }

  template <class Context>
  void PostUpdateImpl(Context const* ctx, int a, double b, QuoteData const& qd)
  {
    std::cout << "Updating..." << std::endl;
  }
};

static_assert(MyIndicator::CanStepOn<Trade>(), "");
static_assert(MyIndicator::CanStepOn<Ask>(), "");
static_assert(!MyIndicator::CanStepOn<Bid>(), "");
static_assert(MyIndicator::CanUpdate(), "");
static_assert(MyIndicator::CanPostUpdate(), "");


// static_assert(MyIndicator::CanCall<step_impl_test, Trade>(), "");
// static_assert(!MyIndicator::CanCall<step_impl_test, Bid>(), "");
// static_assert(MyIndicator::CanCall<step_impl_test, Ask>(), "");

int main()
{
  int a = 13;
  double b = 3.14;
  QuoteData qd{5};
  
  std::unordered_map<std::string, void*> inputs;
  inputs["a"] = &a;
  inputs["b"] = &b;
  inputs["quoteData"] = &qd;
  Context ctx;
  
  MyIndicator ind{ctx, inputs};
  //ind.InitializeInputs(inputs);
  //ind.ValidateInputs();
  ind.Step(Trade{}, &ctx);
  qd.get(0) = 13.13;
  qd.get(2) = 33.22;
  ind.Step(Trade{}, &ctx);
}

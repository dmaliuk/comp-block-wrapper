#include <utility>
#include <tuple>
#include <vector>
#include <iostream>
#include <exception>
#include <unordered_map>
#include <string>

struct Trade
{
  double price;
  int size;
};

void Assert(bool cond)
{
  if (!cond)
    throw std::exception{};
}

template <class Indicator>
class IndicatorWithInputsAndContext
{
public:
  template <class Context, class Inputs>
  IndicatorWithInputsAndContext(Context const& ctx, Inputs const& inputNames)
  {
    Instance().InitializeInputs(inputNames);
    Instance().ValidateInputs();
  }

  Indicator& Instance()
  {
    return static_cast<Indicator&>(*this);
  }

  template <class Context>
  void Step(Trade const & e, Context const * c)
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

class MyIndicator : public IndicatorWithInputsAndContext<MyIndicator>
{
public:
  int* a;
  double* b;
  QuoteData* quoteData;

  inline auto AsTuple() const {
    return std::tie(a, b, quoteData);
  }
    
  template <class F>
  void ForEachInput(F&& f)
  {
    f(0, a);
    f(1, b);
    f(2, quoteData);
  }
  
  constexpr static size_t NumInputs()
  {
    return 3;
  }

  static char const* InputName(int i)
  {
    switch(i)
    {
      case 0:
        return "a";
      case 1:
        return "b";
      case 2:
        return "quoteData";
      default:
        return nullptr;
    }
  }

  template <class Inputs>
  void InitializeInputs(Inputs const& inputNames)
  {
    ForEachInput([&inputNames, this](auto idx, auto& inputPtr) {
        auto name = this->InputName(idx);
        std::cout << "Initializing input " << name << std::endl;
        auto it = inputNames.find(name);
        std::cout << "found pointer " << it->second << std::endl;
        Assert(it != inputNames.end());
        inputPtr = static_cast<std::decay_t<decltype(inputPtr)>>(it->second);
      });
  }
                
  void ValidateInputs()
  {
    ForEachInput([](auto idx, auto& inputPtr) {
        Assert(inputPtr);
      });
  }

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
};

struct Context {};

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
  ind.ValidateInputs();
  ind.Step(Trade{}, &ctx);
  qd.get(0) = 13.13;
  qd.get(2) = 33.22;
  ind.Step(Trade{}, &ctx);
}

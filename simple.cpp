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
    std::cout << "this of IndicatorWithInputsAndContext " << this << std::endl;
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
    // for each input dereference it and // try Indicator::NumInputs()
    StepHelper(e, c, std::make_index_sequence<Indicator::NumInputs()>{});
  }

  template <class Context, size_t... I>
  void StepHelper(Trade const & e, Context const * c, std::index_sequence<I...>)
  {
    //auto& ptrs = Instance().inputPtrs;
    auto ptrs = Instance().TieAsTuple();
    //std::cout << "StepHelper tuple address " << &ptrs << std::endl;

    // std::cout << "StepHelper pointers: " << std::get<0>(ptrs)
    //           << " " << std::get<1>(ptrs) << std::endl;
    Instance().StepImpl(e, c, *std::get<I>(ptrs)...);
  }
  
};

class MyIndicator : public IndicatorWithInputsAndContext<MyIndicator>
{
public:
  //std::tuple<int*, double*> inputPtrs;
  int* a;
  double* b;

  inline auto TieAsTuple() const {
    return std::tie(a, b);
  // }
    

  // static char const* InputName(int i)
  // {
  //   switch(i)
  //   {
  //     case 0:
  //       return "a";
  //     case 1:
  //       return "b";
  //     default:
  //       return nullptr;
  //   }
  // }

  template <class F>
  void ForEachInput(F&& f)
  {
    f(0, std::get<0>(inputPtrs));
    f(1, std::get<1>(inputPtrs));
  }
  
  constexpr static size_t NumInputs()
  {
    return std::tuple_size<decltype(inputPtrs)>::value;
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
        std::cout << "pointer after assignment" << inputPtr << std::endl;
      });

  //   std::cout << "this of MyIndicator " << this << std::endl;

  //   std::cout << "pointers: " << std::get<0>(inputPtrs)
  //             << " " << std::get<1>(inputPtrs) << std::endl;
    std::cout << "InitializeInputs tuple address " << &inputPtrs << std::endl;
  }
                
  void ValidateInputs()
  {
    ForEachInput([](auto idx, auto& inputPtr) {
        Assert(inputPtr);
      });

    std::cout << "ValidateInputs pointers: " << std::get<0>(inputPtrs)
              << " " << std::get<1>(inputPtrs) << std::endl;
  }
    

  using Base = IndicatorWithInputsAndContext<MyIndicator>;
  using Base::Step;

  template <class Context, class Inputs>
  MyIndicator(Context const& ctx, Inputs const& inputNames): Base(ctx, inputNames)
  {
    std::cout << "MyIndicator constructor pointers: " << std::get<0>(inputPtrs)
              << " " << std::get<1>(inputPtrs) << std::endl;
  }

  template <class Context>
  void StepImpl(Trade const& t, Context const* ctx, int a, double b)
  {
    std::cout << "a: " << a << " b: " << b << std::endl;
  }
};

struct Context {};

int main()
{
  int a = 13;
  double b = 3.14;
  
  std::unordered_map<std::string, void*> inputs;
  inputs["a"] = &a;
  inputs["b"] = &b;
  Context ctx;
  
  MyIndicator ind{ctx, inputs};
  //ind.InitializeInputs(inputs);
  ind.ValidateInputs();
  ind.Step(Trade{}, &ctx);
}

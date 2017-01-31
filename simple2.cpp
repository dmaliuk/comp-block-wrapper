#include "indicator.h"


int main()
{
  int a = 13;
  double b = 3.14;
  int const num_exch = 3;
  QuoteData qd{num_exch};
  
  std::unordered_map<std::string, void*> inputs;
  inputs["a"] = &a;
  inputs["b"] = &b;
  inputs["quoteData"] = &qd;
  Context ctx;
  MyIndicator ind{ctx, inputs};

  for (int i = 0; i < num_exch; ++i)
  {
    Bid bid{90. + i, 100 + 10 * i, i};
    ind.Step(bid, &ctx);
    Ask ask{100. + i, 100 + 10 * i, i};
    ind.Step(ask, &ctx);
  }

  {
    Trade trade{90., 100, 0};
    ind.Step(trade, &ctx);
  }

  {
    Trade trade{101., 110, 1};
    ind.Step(trade, &ctx);
  }

  {
    Trade trade{92., 200, 2};
    ind.Step(trade, &ctx);
  }

  ind.Update(&ctx);

  std::cout << "value: " << ind.output << std::endl;
    
}

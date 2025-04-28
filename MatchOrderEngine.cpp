#include "MatchOrderEngine.h"

namespace MarketData
{
MatchOrderEngine::MatchOrderEngine()
{
}

void MatchOrderEngine::Initialize()
{
    order_book = std::make_shared<OrderBook>();
}

bool MatchOrderEngine::AddOrder(OrderPtr order)
{
    return order_book->AddOrder(order);
}

bool MatchOrderEngine::UpdateOrder(const ModifyOrder& order)
{
    return true;
}

void MatchOrderEngine::PrintBook()
{

}
    
}
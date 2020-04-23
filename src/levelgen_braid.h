#pragma once
#include <level.h>
#include <memory>

namespace levelgen::braid
{

class BraidLevelGenerator : public GeneratorAlgorithm
{
  public:
    std::unique_ptr<Level> Generate(Size size) override;
};

} // namespace levelgen::braid
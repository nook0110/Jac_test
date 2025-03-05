#pragma once

#include <cstddef>
#include <memory>
#include <string_view>
#include <vector>

namespace phc
{
class PHCWrapper
{
 public:
  void Clear() const;
  void InitializeNumberOfPolynomials(size_t n) const;
  void InsertPolynomial(std::string_view polynomial, size_t idx) const;
  size_t GetAmountOfSymbols() const;
  bool IsSquare() const;
  void SetSymbols(const std::vector<std::string>& symbols);

  struct Root
  {
    size_t multiplicity;
    struct Complex
    {
      std::string real;
      std::string imag;
      bool operator==(const Complex&) const = default;
    };
    std::vector<Complex> data;

    bool operator==(const Root&) const = default;
  };
  Root ParseRoot(std::string root_string) const;
  std::vector<Root> Solve() const;
  size_t GetTotalDegree() const;
  size_t GetBezoutNumber() const;

  static PHCWrapper& GetInstance();

 private:
  PHCWrapper();
  PHCWrapper(PHCWrapper&&) = delete;
  PHCWrapper(const PHCWrapper&) = delete;
  PHCWrapper& operator=(PHCWrapper&&) = delete;
  PHCWrapper& operator=(const PHCWrapper&) = delete;
  ~PHCWrapper();

  class PHCWrapperImplementation;
  std::unique_ptr<PHCWrapperImplementation> implementation_;
};

}  // namespace phc
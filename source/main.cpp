#include <cln/float.h>
#include <cln/floatformat.h>
#include <ginac/ex.h>
#include <ginac/ginac.h>
#include <ginac/numeric.h>
#include <ginac/power.h>
#include <glog/logging.h>
#include <phc/phcpack4c.h>
#include <phc/solcon.h>
#include <phc/syscon.h>
#include <phc/unisolvers.h>

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>

#include "checker.hpp"
#include "mysql.hpp"
#include "polynomial.hpp"
#include "source/phc_wrapper.hpp"
#include "source/point.hpp"
#include "source/solver.hpp"
#include "symbol.hpp"

void TestMap(const Map& map, FunctionDatabase& db);
int main(int argc, char* argv[])
{
  google::SetStderrLogging(google::LogSeverity::INFO);
  google::InitGoogleLogging(argv[0]);
  cln::cl_inhibit_floating_point_underflow = true;

  bool user_input = argc > 0;
  if (user_input)
  {
    size_t dimensions;
    std::cin >> dimensions;
    Symbols::FitInto(dimensions);

    std::vector<Polynomial> polynomials(dimensions);

    std::string input;
    std::getline(std::cin, input);
    for (auto& poly : polynomials)
    {
      std::string s;
      std::getline(std::cin, s);
      poly = GiNaC::ex(s, Symbols::GetSymbolsList());
    }

    Map map(std::move(polynomials));

    LOG(INFO) << "Map has contraction: " << map.HasContraction();

    return 0;

    Point point = Point::GenerateRandom(dimensions);
    point = map.Image(
        GeneratePointWithUnitJacobian(map, Point::GenerateRandom(dimensions))
            .value_or(point));

    Checker checker;
    checker.SetMap(&map);
    if (!checker.TestPoint(point))
    {
      LOG(WARNING) << "Point did not pass the Jacobian test.";
      LOG(WARNING) << "Map was: " << map.ToStr();
      LOG(WARNING) << "Point was: " << point.ToStr();
    }
  }

  FunctionDatabase db;

  size_t dimensions = 2;

  std::vector<Polynomial> monomials;
  size_t degree_count = 3;
  for (size_t i = 0; i < degree_count; ++i)
  {
    std::vector<size_t> degrees = {i, degree_count - i};
    monomials.push_back(GenerateMonomial(degrees));
  }

  std::vector<Complex> coefficients = {-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5};

  for (size_t seed = 10; seed < 1'000'000; ++seed)
  {
    LOG(INFO) << "Processing seed: " << seed;

    std::vector<Polynomial> generated_polynomials(dimensions);
    for (size_t i = 0; i < dimensions; ++i)
    {
      generated_polynomials[i] = GeneratePolynomial(
          monomials, coefficients, i * i * seed + seed * seed + i);
    }

    Map map(std::move(generated_polynomials));

    if (db.MapExists(map))
    {
      continue;
    }

    try
    {
      TestMap(map, db);
    }
    catch (const std::exception& e)
    {
      LOG(ERROR) << "Exception occurred: " << e.what();
      db.InsertCheckResult(CheckResult(&map, CheckResult::Error{e.what()}));
    }
  }
}

void TestMap(const Map& map, FunctionDatabase& db)
{
  const size_t dimensions = map.GetDimensions();
  size_t iterations = 10;
  for (size_t i = 0; i < iterations; ++i)
  {
    Point point = Point::GenerateRandom(dimensions);
    point = map.Image(
        GeneratePointWithUnitJacobian(map, Point::GenerateRandom(dimensions))
            .value_or(point));

    Checker checker;
    checker.SetMap(&map);
    if (auto result = checker.TestPoint(point))
    {
      LOG(WARNING) << "Point did not pass the Jacobian test.";
      LOG(WARNING) << "Map was: " << map.ToStr();
      LOG(WARNING) << "Point was: " << point.ToStr();

      db.InsertCheckResult(
          CheckResult(&map, CheckResult::Failed{point, *result}));
      return;
    }
  }

  db.InsertCheckResult(CheckResult(&map, CheckResult::Passed{iterations}));
}
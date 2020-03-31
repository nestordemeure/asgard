#include "tests_general.hpp"
#include "timer.hpp"
#include <algorithm>
#include <random>
#include <sstream>
// function that does nothing productive, for testing the timer
double shuffle_random(int const num_items)
{
  assert(num_items > 0);
  std::random_device rd;
  std::mt19937 mersenne_engine(rd());
  std::uniform_real_distribution<double> dist(0.1, 1.0);
  auto const gen = [&dist, &mersenne_engine]() {
    return dist(mersenne_engine);
  };

  std::vector<double> items(num_items);
  std::generate(items.begin(), items.end(), gen);
  std::shuffle(items.begin(), items.end(), mersenne_engine);
  return items[0];
}

TEST_CASE("test recorder")
{
  timer::recorder record;
  int const items_to_gen       = 1000000;
  int const iterations         = 10;
  std::string const identifier = "waste_time";
  for (int i = 0; i < iterations; ++i)
  {
    double const val = record(shuffle_random, identifier, items_to_gen);
    assert(val > 0.0); // to avoid comp. warnings
  }
  std::string const report = record.report();
  std::stringstream s1(report.substr(report.find("avg: ")));

  std::string s;
  s1 >> s;
  double avg;
  s1 >> avg;

  std::stringstream s2(report.substr(report.find("min: ")));
  s2 >> s;
  double min;
  s2 >> min;

  std::stringstream s3(report.substr(report.find("max: ")));
  s3 >> s;
  double max;
  s3 >> max;

  std::stringstream s4(report.substr(report.find("med: ")));
  s4 >> s;
  double med;
  s4 >> med;

  std::stringstream s5(report.substr(report.find("calls: ")));
  s5 >> s;
  int calls;
  s5 >> calls;

  auto const &times = record.get_times(identifier);

  SECTION("avg")
  {
    double sum = 0.0;
    for (double const &time : times)
    {
      sum += time;
    }
    double const gold_average = sum / times.size();
    REQUIRE(avg == gold_average);
  }

  SECTION("min/max")
  {
    double gold_min = std::numeric_limits<double>::max();
    double gold_max = std::numeric_limits<double>::min();

    for (double const &time : times)
    {
      gold_min = time < gold_min ? time : gold_min;
      gold_max = time > gold_max ? time : gold_max;
    }

    REQUIRE(min == gold_min);
    REQUIRE(max == gold_max);
  }

  SECTION("med")
  {
    std::vector<double> time_copy(times);
    std::sort(time_copy.begin(), time_copy.end());
    auto const mid        = time_copy.size() / 2;
    double const gold_med = (time_copy.size() % 2 == 0)
                                ? (time_copy[mid] + time_copy[mid - 1]) / 2
                                : time_copy[mid];
    REQUIRE(med == gold_med);
  }

  SECTION("count") { REQUIRE(calls == static_cast<int>(times.size())); }
}

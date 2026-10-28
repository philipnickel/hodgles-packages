// Consumer smoke build: reads a tagged union and prints its JSON Schema. An
// alternative with an rfl::DefaultVal member crashed to_schema before
// reflect-cpp-tagged-union-defaults.patch.
#include <rfl.hpp>
#include <rfl/json.hpp>

#include <iostream>

struct Sine {
  using Tag = rfl::Literal<"sine">;
  rfl::DefaultVal<double> phase = 0.0;
  double frequency;
};
struct Constant {
  using Tag = rfl::Literal<"constant">;
  rfl::DefaultVal<double> scale = 1.0;
};
using Function1 = rfl::TaggedUnion<"type", Sine, Constant>;

int main() {
  const auto f = rfl::json::read<Function1, rfl::NoExtraFields>(R"({"type": "sine", "frequency": 2})");
  if (!f) {
    std::cerr << f.error().what() << "\n";
    return 1;
  }
  std::cout << rfl::json::write(*f) << "\n" << rfl::json::to_schema<Function1>() << "\n";
  return 0;
}

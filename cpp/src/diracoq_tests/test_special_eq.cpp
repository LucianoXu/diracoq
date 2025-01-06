#include <gtest/gtest.h>

#include "diracoq.hpp"

using namespace ualg;
using namespace std;
using namespace diracoq;

TEST(DiracoqSpecialEq, Delta) {
    Prover prover;
    prover.process("Var T : INDEX. Var a : BASIS[T]. Var b : BASIS[T].");
    EXPECT_TRUE(prover.check_eq("DELTA[a, b]", "DELTA[b, a]"));
}

TEST(DiracoqSpecialEq, SUM_SWAP) {
    Prover prover;

    prover.process(
        R"(
            Var T : INDEX.
            Var M : INDEX.
        )");

    EXPECT_TRUE(prover.check_eq(
        "Sum i in USET[T], Sum j in USET[M], 1",
        "Sum j in USET[M], Sum i in USET[T], 1"
    ));
}
#include <gtest/gtest.h>

#include "ualg.hpp"

using namespace ualg;
using namespace std;

///////////////////
// NormalTerm
TEST(TestTerm, get_term_size) {
    TermBank<string> bank{};

    auto t = bank.get_normal_term("t", {});
    auto s = bank.get_normal_term("s", {t});
    auto r = bank.get_normal_term("r", {t});
    auto a = bank.get_normal_term("&", {s, r});

    EXPECT_EQ(a->get_term_size(), 4);
}

TEST(TestTerm, get_subterm) {
    TermBank<string> bank{};

    auto t = bank.get_normal_term("t", {});
    auto s = bank.get_normal_term("s", {t});
    auto r = bank.get_normal_term("r", {t});
    auto a = bank.get_normal_term("&", {s, r});

    auto subterm = a->get_subterm({0});
    EXPECT_EQ(subterm, s);
}


TEST(TestTermBank, HashConsing) {
    TermBank<string> bank{};

    // for variables
    auto t1 = bank.get_normal_term("t", {});
    auto t2 = bank.get_normal_term("t", {});
    EXPECT_EQ(t1, t2);

    // for normal terms
    auto s = bank.get_normal_term("s", {});
    auto a = bank.get_normal_term("&", {t1, s});
    auto b = bank.get_normal_term("&", {t2, s});
    EXPECT_EQ(a, b);
}


TEST(TestTermBank, construct_term) {
    TermBank<string> bank{};

    // for variables
    auto s1 = NormalTerm<string>("s", {});
    auto s2 = NormalTerm<string>("s", {});
    auto t1 = NormalTerm<string>("t", {&s1, &s2});
    EXPECT_EQ(t1.get_term_size(), 3);

    // for normal terms
    auto t2 = bank.construct_term(t1);
    EXPECT_EQ(t2->get_term_size(), 2);
}

TEST(TestTermBank, replace_term) {
    TermBank<string> bank{};

    auto s = bank.get_normal_term("s", {});
    auto t = bank.get_normal_term("t", {});
    auto a = bank.get_normal_term("&", {s, s});

    // replacement
    std::map<const Term<string>*, const Term<string>*> mapping;
    mapping[s] = t;
    auto actual_res = bank.replace_term(a, mapping);

    auto expected_res = bank.get_normal_term("&", {t, t});

    EXPECT_EQ(actual_res, expected_res);
}

TEST(TestTermBank, replace_term_pos) {
    TermBank<string> bank{};

    auto s = bank.get_normal_term("s", {});
    auto t = bank.get_normal_term("t", {});
    auto a = bank.get_normal_term("&", {s, s});

    // replacement
    auto actual_res = bank.replace_term(a, {0}, t);

    auto expected_res = bank.get_normal_term("&", {t, s});

    EXPECT_EQ(actual_res, expected_res);
}

///////////////////////
// C Terms
TEST(TestCTerm, operator_eq) {
    TermBank<string> bank{};

    auto s = bank.get_normal_term("s", {});
    auto t = bank.get_normal_term("t", {});
    auto a = bank.get_c_term("&", {{s, 1}, {t, 2}});
    auto b = bank.get_c_term("&", {{t, 2}, {s, 1}});
    EXPECT_EQ(a, b);
}

///////////////////////
// AC Terms
TEST(TestACTerm, operator_eq) {
    TermBank<string> bank{};

    auto s = bank.get_ac_term("s", {});
    auto t = bank.get_ac_term("t", {});
    auto a = bank.get_ac_term("&", {{s, 1}, {t, 1}});
    auto actual_res = bank.get_ac_term("&", {{a, 1}, {t, 1}});
    auto expected_res = bank.get_ac_term("&", {{s, 1}, {t, 2}});
    EXPECT_EQ(actual_res, expected_res);
}

#include <iostream>
#include <sstream>

#include "../src/json.h"

using namespace std;
using namespace json;

int ok_cnt = 0, fail_cnt = 0;

void ok() { ok_cnt++; }

void fail() { fail_cnt++; }

void fail(std::string msg) {
    cout << "fail: " + msg;
    fail();
}

void test(bool assertion, string msg) {
    if (assertion)
        ok();
    else
        fail(msg);
}

template <class A, class B>
void eq(A a, B b) {
    if (a == b) {
        ok();
    } else {
        fail();
        cout << "fail: " << a << " != " << b << endl;
    }
}

template <class A, class B>
void neq(A a, B b) {
    if (a != b) {
        ok();
    } else {
        fail();
        cout << "fail: " << a << " != " << b << endl;
    }
}

void test_array() {
    auto array = Array<vector<int>>({2, 3, 4});
    eq(array(0), 2);
    eq(array(1), 3);
    eq(array(2), 4);

    int a[] = {7, 9, 8};
    array = Array(a, a + std::size(a));
    eq(array(0), 7);
    eq(array(1), 9);
    eq(array(2), 8);
}

void test_iterator() {
    JSONObject json, sum = 0;
    json("a") = 10;
    json("b") = 7.98;

    for (auto &field : json) {
        field.second = field.second + 1.0;
    }
    for (auto field : json) {
        sum = sum + field.second;
    }

    eq(sum, 19.98);

    auto array = Array();
    array(2) = 2;
    array(0) = 0;

    cout << array << endl;

    for (auto &elem : array) cout << elem.second << endl;
}

int main() {
    test_array();
    test_iterator();

    JSONParser parser;
    auto json = load("test.json");
    auto json2 = JSONObject{};

    eq(json, json);
    eq(json, json2);

    eq(json("_"), 0);
    neq(string(json("_")), "");

    json("a") = 12;
    eq(json("a"), 12);

    auto cjo = json;
    eq(cjo("_"), 0);
    eq(cjo("_")("_"), 0);

    eq(json("i"), 1);
    neq(json("i"), 1.5);

    test(json("x") != 11, "x");
    test(json("x") == 11.5, "x");
    test(+json("x") == +11.5, "x");
    test(-json("x") == -11.5, "x");
    test(json("x") + 1 == 12.5, "x");
    test(json("x") - 1 == 10.5, "x");

    JSONObject num = 11.5;
    test(num == 11.5, "num");
    test(num == json("x"), "num");
    test(string(num) == "11.5", "num");

    JSONObject strnum = "17.70";
    test(strnum == 17.7, "strnum");
    test(string(strnum) == "17.70", "strnum");

    string s = json("s");
    test(s == "text", "s");
    string text = "text";
    test(string(json("s")) == "text", "s");
    // test(json("s") + "" == "text", "s");

    cout << ok_cnt << " ok, " << fail_cnt << " fail";
}
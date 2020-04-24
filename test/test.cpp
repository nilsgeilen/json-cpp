#include <iostream>

#include "../src/json.h"

using namespace std;
using namespace json;

void test(bool assertion, string msg) {
    if (assertion)
        cout << "ok" << endl;
    else
        cout << "fail: " << msg << endl;
}

int main() {
    JSONParser parser;
    auto json = parser.parse("test.json");

    test(json("_") == 0, "_");
    cout << string(json("_")) << endl;
    test(string(json("_")) == "", "_");

    json("a")=12;

    auto cjo = json;
    test(cjo("_") == 0, "const");
    test(cjo("_")("_") == 0, "const");
    cout << cjo << endl;

    test(json("i") == 1, "i");

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
    string text= "text";
    test(string(json("s")) == "text", "s");
}
#include "json.h"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace json {
using namespace std;

namespace {
inline JSONObject &nil() {
    static JSONObject nil{"__null__", NIL};
    return nil;
}
}  // namespace

JSONObject &JSONObject::operator()(const std::string &key) {
    if (type == STR || type == NUM || type == NIL) return nil();
    if (type == ARRAY) {
        size_t pos;
        std::stringstream{key} >> pos;
        if (pos >= size()) val = std::to_string(pos + 1);
    }
    return fields[key];
}

const JSONObject &JSONObject::operator()(const std::string &key) const {
    if (type == STR || type == NUM || type == NIL) return nil();
    if (type == ARRAY) {
        size_t pos;
        std::stringstream{key} >> pos;
        if (pos < size()) {
            auto field = fields.find(key);
            if (field == fields.end())
                return nil();
            else
                return field->second;
        }
    }
    return fields.at(key);
}

void JSONPrinter::print(std::ostream &stream, const JSONObject &jo, int tab) {
    switch (jo.get_type()) {
        case NIL:
            stream << "null";
            break;
        case NUM:
            stream << double(jo);
            break;
        case STR:
            stream << format.DOUBLE_QUOTE << jo.str() << format.DOUBLE_QUOTE;
            break;
        case OBJ: {
            stream << format.OBJ_BEGIN;
            bool first = true;
            for (auto field : jo) {
                if (first) {
                    first = false;
                } else {
                    stream << ',';
                    if (verbose) stream << ' ';
                }
                if (verbose) {
                    stream << '\n';
                    for (int i = 0; i < tab + 1; i++) stream << '\t';
                }
                stream << field.first;
                if (verbose) stream << ' ';
                stream << format.ASSIGN;
                if (verbose) stream << ' ';
                print(stream, field.second, tab + 1);
            }
            if (!first && verbose) {
                stream << '\n';
                for (int i = 0; i < tab; i++) stream << '\t';
            }
            stream << format.OBJ_END;
            break;
        }
        case ARRAY: {
            stream << format.ARRAY_BEGIN;
            int size = jo.num<int>();
            bool first = true;
            for (int i = 0; i < size; i++) {
                if (first) {
                    first = false;
                } else {
                    stream << ',';
                    if (verbose) stream << ' ';
                }
                print(stream, jo(i), tab + 1);
            }
            stream << format.ARRAY_END;
        }
    }
}

static void tokenize(std::string source, std::queue<std::string> &tokens,
                     vector<char> delims = {'=', ':', '{', '}', ',', ';', '[',
                                            ']'}) {
    stringstream wordstream;
    bool singlequote = false, doublequote = false, braced = false;
    ;
    for (char c : source) {
        if (!singlequote && !braced && c == '"') {
            doublequote = !doublequote;
            string word = wordstream.str();
            wordstream.str("");
            if (word.size()) tokens.push(word);
            tokens.push(string{c});
        } else if (!doublequote && !braced && c == '\'') {
            singlequote = !singlequote;
            string word = wordstream.str();
            wordstream.str("");
            if (word.size()) tokens.push(word);
            tokens.push(string{c});
        } else if (!doublequote && !singlequote && !braced && c == '(') {
            braced = true;
            string word = wordstream.str();
            wordstream.str("");
            wordstream << '(';
            if (word.size()) tokens.push(word);
        } else if (braced && c == ')') {
            braced = false;
            wordstream << ')';
            string word = wordstream.str();
            wordstream.str("");
            if (word.size()) tokens.push(word);
        } else if (!doublequote && !singlequote && !braced &&
                   find(begin(delims), end(delims), c) != end(delims)) {
            string word = wordstream.str();
            wordstream.str("");
            if (word.size()) tokens.push(word);
            tokens.push(string{c});
        } else if ((c != ' ' && c != '\n' && c != '\t') || singlequote ||
                   doublequote || braced) {
            wordstream << c;
        }
    }
    string word = wordstream.str();
    if (size(word)) tokens.push(word);
}

std::string JSONParser::next_token(std::istream &source, bool pop) {
    while (tokens.empty()) {
        string line;
        getline(source, line);
        tokenize(line, tokens);
    }
    auto token = tokens.front();
    if (pop) tokens.pop();
    return token;
}

JSONObject JSONParser::parse(std::istream &source) {
    auto token = next_token(source);
    if (token[0] == format.OBJ_BEGIN) {
        JSONObject jo;
        while (true) {
            token = next_token(source);
            if (token[0] == format.OBJ_END) break;
            if (token[0] == format.COMMA) continue;
            if (token[0] == format.DOUBLE_QUOTE) {
                token = next_token(source);
                next_token(source);
            }
            next_token(source);
            jo(token) = parse(source);
        }
        return jo;
    } else if (token[0] == format.ARRAY_BEGIN) {
        JSONObject array{ARRAY};
        int i = 0;
        while (true) {
            token = next_token(source, false);
            if (token[0] == format.ARRAY_END) {
                next_token(source);
                break;
            }
            if (token[0] == format.COMMA) {
                next_token(source);
                continue;
            }
            array(i) = parse(source);
            i++;
        }
        return array;
    } else if (token[0] == format.DOUBLE_QUOTE) {
        token = next_token(source);
        next_token(source);
        return JSONObject{token};
    } else {
        return JSONObject{token, NUM};
    }
}

JSONObject JavaConfigParser::parse(std::istream &source) {
    JSONObject jo;
    string prop, val;
    while (getline(source, prop, delim)) {
        if (getline(source, val)) {
            jo(prop) = val;
        }
    }
    return jo;
}

JSONObject load(std::string path) {
    auto ending = path.substr(path.find_last_of('.'));

    ifstream file{path};
    JSONObject obj;

    if (ending == "properties" || ending == "config") {
        obj = JavaConfigParser{}.parse(file);
    } else {
        obj = JSONParser{}.parse(file);
    }

    file.close();
    return obj;
}
}  // namespace json
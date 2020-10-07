
#include <map>
#include <queue>
#include <sstream>
#include <string>

#pragma once

namespace json {

namespace {
template <class T>
inline std::string to_string(T num) {
    std::stringstream strstr;
    strstr << num;
    return strstr.str();
}
}  // namespace

enum Type { NIL,
            STR,
            NUM,
            OBJ,
            ARRAY };

class JSONObject {
    Type type;
    std::string val;
    std::map<std::string, JSONObject> fields;

   public:
    JSONObject(decltype(type) t = OBJ)
        : val{t == OBJ ? "__object__" : t == STR ? "" : "0"}, type{t} {}

    JSONObject(std::string val, decltype(type) t = STR) : val{val}, type{t} {}

    JSONObject(const char *val, decltype(type) t = STR) : val{val}, type{t} {}

    template <class Num>
    JSONObject(Num num_val) : val{to_string(num_val)}, type{NUM} {}

    JSONObject &operator()(const std::string &key);

    const JSONObject &operator()(const std::string &key) const;

    JSONObject &operator()(size_t key) { return (*this)(std::to_string(key)); }

    const JSONObject &operator()(size_t key) const {
        return (*this)(std::to_string(key));
    }

    template <class K, class V>
    V operator()(K key, V def) const {
        if (has(key))
            return (V)(*this)(key);
        return def;
    }

    Type get_type() const { return type; }

    std::string str() const {
        if (type == OBJ) return "{" + std::to_string(size_t(this)) + "}";
        if (type == ARRAY) return "[" + std::to_string(size_t(this)) + "]";
        return val;
    }

    operator std::string() const { return str(); }

    template <class Num>
    Num num() const {
        std::stringstream ss(val);
        Num t;
        ss >> t;
        return t;
    }

    operator double() const { return num<double>(); }

    size_t size() const {
        if (type == ARRAY) return num<size_t>();
        return 0;
    }

    auto begin() const { return fields.begin(); }

    auto end() const { return fields.end(); }

    auto begin() { return fields.begin(); }

    auto end() { return fields.end(); }

    std::vector<std::string> keys() const {
        std::vector<std::string> keys;
        for (const auto &[key, _] : fields) {
            keys.push_back(key);
        }
        return keys;
    }

    bool has(std::string key) const {
        for (const auto &[k, _] : fields) {
            if (k == key)
                return true;
        }
        return false;
    }

    template <class Num>
    std::vector<Num> to_num_array() const {
        std::vector<Num> array;
        const size_t s = size();
        for (size_t i = 0; i < s; i++) {
            array.push_back((*this)(i).num<Num>());
        }
        return array;
    }

    std::vector<std::string> to_str_array() const {
        std::vector<std::string> array;
        const size_t s = size();
        for (size_t i = 0; i < s; i++) {
            array.push_back((*this)(i).str());
        }
        return array;
    }
};

inline JSONObject Array(size_t size = 0) {
    return {std::to_string(size), ARRAY};
}

template <class I>
JSONObject Array(I begin, I end) {
    JSONObject array = Array();
    for (; begin < end; begin++) {
        array(array.size()) = *begin;
    }
    return array;
}

template <class T>
JSONObject Array(T data) {
    return Array(std::begin(data), std::end(data));
}

struct JSONFormat {
    char ASSIGN = ':', ARRAY_BEGIN = '[', ARRAY_END = ']', OBJ_BEGIN = '{',
         OBJ_END = '}', DOUBLE_QUOTE = '"', COMMA = ',';
};

class JSONPrinter {
   public:
    bool verbose = true;
    JSONFormat format;
    void print(std::ostream &stream, const JSONObject &jo, int tab = 0);
};

inline std::ostream &operator<<(std::ostream &stream, const JSONObject &jo) {
    JSONPrinter{}.print(stream, jo);
    return stream;
}

class JSONParser {
    std::queue<std::string> tokens;

   public:
    JSONFormat format;

    JSONObject parse(std::istream &source);

   private:
    std::string next_token(std::istream &source, bool pop = true);
};

class JavaConfigParser {
   public:
    char delim = '=';

    JSONObject parse(std::istream &source);
};

JSONObject load(std::string path);

template <class T, class Serializor>
void serialize(T &o, Serializor s) {
    o.serialize(s);
}

struct SerializorFromJSON {
    JSONObject &jo;

    template <class T>
    void assoc(T &t, std::string key) {
        t = (T)jo(key);
    }

    template <class T>
    void vassoc(T &array, std::string key) {
        auto &child = jo(key);
        array.resize(std::size(child));
        for (int i = 0; i < std::size(array); i++) {
            serialize(array[i], SerializorFromJSON{child(i)});
        }
    }

    // template <class T, class F>
    // void assoc(std::vector<T> &array, F *f) {
    //     array.resize(std::size(jo));
    //     for (int i = 0; i < std::size(array); i++) {
    //         f(SerializorFromJSON{jo[i]}, array[i]);
    //     }
    // }
};

struct SerializorToJSON {
    json::JSONObject &jo;
    template <class T>
    void assoc(const T &t, std::string key) {
        jo(key) = t;
    }

    // template <class T, class F>
    // void assoc(const std::vector<T> &array, F *f) {
    //     // jo.type = JSONObject::ARRAY;
    //     // jo.val = std::to_string(std::size(array));
    //     jo = Array();
    //     for (int i = 0; i < std::size(array); i++) {
    //         f(SerializorToJSON{jo[i]}, array[i]);
    //     }
    // }
};

template <class T>
JSONObject tojson(T &t) {
    JSONObject jo;
    t.serialize(SerializorToJSON{jo});
    return jo;
}

template <class T>
JSONObject vectojson(std::vector<T> &a) {
    JSONObject jo = Array(std::size(a));
    for (int i = 0; i < std::size(a); i++)
        a[i].serialize(SerializorToJSON{jo(i)});
    return jo;
}

template <class T>
T fromjson(JSONObject &jo) {
    T t;
    t.serialize(SerializorFromJSON{jo});
    return t;
}

template <class T>
std::vector<T> vecfromjson(JSONObject &jo) {
    std::vector<T> vec(std::size(jo));
    for (int i = 0; i < std::size(vec); i++)
        vec[i].serialize(SerializorFromJSON{jo(i)});
    return vec;
}

}  // namespace json
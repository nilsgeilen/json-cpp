
#include <map>
#include <queue>
#include <sstream>
#include <string>

#pragma once

namespace json {

class JSONObject {
    std::string val;
    std::map<std::string, JSONObject> fields;

   public:
    enum { STR, NUM, OBJ, ARRAY } type;

    JSONObject(decltype(type) t = OBJ)
        : val(t == OBJ ? "__object__" : t == STR ? "" : "0"), type(t) {}

    JSONObject(std::string val, decltype(type) t = STR) : val(val), type(t) {}

    JSONObject(const char *val, decltype(type) t = STR) : val(val), type(t) {}

    template <class Num>
    JSONObject(Num num_val) : type(NUM) {
        std::stringstream strstr;
        strstr << num_val;
        val = strstr.str();
    }

    JSONObject &operator()(const std::string &key) {
        if (type == ARRAY) {
            int pos;
            std::stringstream{key} >> pos;
            if (pos >= num<int>()) val = std::to_string(pos + 1);
        }
        return fields[key];
    }

    const JSONObject &operator()(const std::string &key) const {
        return fields.at(key);
    }

    JSONObject &operator()(size_t key) { return (*this)(std::to_string(key)); }

    const JSONObject &operator()(size_t key) const {
        return (*this)(std::to_string(key));
    }

    std::string str() const { return val; }

    operator std::string() const { return str(); }

    template <class T>
    T num() const {
        std::stringstream ss(val);
        T t;
        ss >> t;
        return t;
    }

    //  template <class Num>
    //  operator Num() const {
    //      return num<Num>();
    //  }

    operator double() const { return num<double>(); }

    size_t size() const { return num<size_t>(); }

    template <class T>
    std::vector<T> num_array() const {
        std::vector<T> array;
        const int size = num<int>();
        for (int i = 0; i < size; i++) {
            array.push_back((*this)(i).num<T>());
        }
        return array;
    }

    std::vector<std::string> str_array() const {
        std::vector<std::string> array;
        const int size = num<int>();
        for (int i = 0; i < size; i++) {
            array.push_back((*this)(i).str());
        }
        return array;
    }

    friend class JSONPrinter;
};

inline JSONObject Array(size_t size = 0) {
    return {std::to_string(size), JSONObject::ARRAY};
}

template <class T>
JSONObject Array(std::vector<T> data) {
    JSONObject array = Array(std::size(data));
    for (unsigned int i = 0; i < std::size(data); i++) {
        array(i) = data[i];
    }
    return array;
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
    JSONPrinter printer;
    printer.print(stream, jo);
    //  stream << jo.str();
    return stream;
}

class JSONParser {
    std::queue<std::string> tokens;

   public:
    JSONFormat format;

    JSONObject parse(std::istream &source);
    JSONObject parse(std::string path);

   private:
    std::string next_token(std::istream &source, bool pop = true);
};

class JavaConfigParser {
   public:
    std::istream &source;
    char delim = '=';

    JSONObject parse() {
        using namespace std;

        JSONObject jo;
        string prop, val;
        while (getline(source, prop, delim)) {
            if (getline(source, val)) {
                jo(prop) = val;
            }
        }
        return jo;
    }
};

template <class T, class Serializor>
void serialize(T &o, Serializor s) {
    o.serialize(s);
}

struct SerializorFromJSON {
    JSONObject &jo;

    template <class T>
    void assoc(T &t, std::string key) {
        // auto &child = jo[key];

        // t = (T)child;

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
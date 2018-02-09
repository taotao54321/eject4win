#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>

namespace util {

namespace noncopyable_ {
    class noncopyable {
    protected:
        constexpr noncopyable() = default;
        ~noncopyable() = default;
        noncopyable(const noncopyable&) = delete;
        noncopyable& operator=(const noncopyable&) = delete;
    };
}
using noncopyable = noncopyable_::noncopyable;

inline bool ascii_islower(char c) {
    return 'a' <= c && c <= 'z';
}

inline bool ascii_isupper(char c) {
    return 'A' <= c && c <= 'Z';
}

inline bool ascii_isalpha(char c) {
    return ascii_islower(c) || ascii_isupper(c);
}

inline char ascii_tolower(char c) {
    return ascii_isupper(c) ? c+0x20 : c;
}

inline char ascii_toupper(char c) {
    return ascii_islower(c) ? c-0x20 : c;
}

inline bool str_startswith(const std::string& s, const std::string& prefix) {
    return s.find(prefix) == 0;
}

inline void warn(const std::string& msg) {
    std::fputs(msg.c_str(), stderr);
    std::putc('\n', stderr);
}

inline void error(const std::string& msg) {
    warn(msg);
    std::exit(1);
}

inline void print(const std::string& msg) {
    std::fputs(msg.c_str(), stdout);
}

inline void println(const std::string& msg) {
    print(msg);
    std::putc('\n', stdout);
}

} // namespace util

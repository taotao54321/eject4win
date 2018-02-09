#pragma once

#include <stdexcept>

class EjectError : public std::runtime_error {
public:
    explicit EjectError(const std::string& what_arg) : std::runtime_error(what_arg) {}
    explicit EjectError(const char*        what_arg) : std::runtime_error(what_arg) {}
};

class OpticalDrive {
public:
    static char find_first();

    explicit OpticalDrive(char letter);

    void tray_open() const;
    void tray_close() const;
    void tray_toggle() const;

private:
    char letter_;
};

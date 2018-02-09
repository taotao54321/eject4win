#include <exception>
#include <string>

#include "driver.hpp"
#include "util.hpp"

namespace {

struct Args {
    using Action = int (*)(const Args& args);
    Action action;
    char letter;
};

int action_help(const Args& /*args*/) {
    util::print(1 + R"HELP(
Usage: eject [options] [drive]

Options:
  -h, --help           display command usage and exit
  -d, --default        display default drive
  -t, --trayclose      close tray
  -T, --traytoggle     toggle tray
)HELP");

    return 1;
}

int action_default(const Args& /*args*/) {
    char letter = OpticalDrive::find_first();
    std::string dev = letter != 0 ? std::string{letter} : "(none)";
    util::println("eject: default device: `" + dev + "'");

    return 0;
}

int action_trayopen(const Args& args) {
    char letter = args.letter ? args.letter : OpticalDrive::find_first();
    if(letter == 0) {
        util::warn("no optical drive found");
        return 1;
    }

    OpticalDrive drive(letter);
    drive.tray_open();

    return 0;
}

int action_trayclose(const Args& args) {
    char letter = args.letter ? args.letter : OpticalDrive::find_first();
    if(letter == 0) {
        util::warn("no optical drive found");
        return 1;
    }

    OpticalDrive drive(letter);
    drive.tray_close();

    return 0;
}

int action_traytoggle(const Args& args) {
    char letter = args.letter ? args.letter : OpticalDrive::find_first();
    if(letter == 0) {
        util::warn("no optical drive found");
        return 1;
    }

    OpticalDrive drive(letter);
    drive.tray_toggle();

    return 0;
}

// accept "X", "X:"
bool parse_drive(const std::string& s, char& letter) {
    if(s.size() >= 3) return false;
    if(!util::ascii_isalpha(s[0])) return false;
    if(s.size() == 2 && s[1] != ':') return false;

    letter = util::ascii_toupper(s[0]);
    return true;
}

Args parse_args(int argc, const char* const* argv) {
    Args res;
    res.action = action_trayopen;
    res.letter = 0;

    for(int i = 1; i < argc; ++i) {
        std::string s = argv[i];

        if(s.empty()) {
            util::warn("empty argument");
            res.action = action_help;
            break;
        }

        if(util::str_startswith(s, "--")) {
            std::string name = s.substr(2);
            if(name == "help") {
                res.action = action_help;
                break;
            }
            else if(name == "default") {
                res.action = action_default;
            }
            else if(name == "trayclose") {
                res.action = action_trayclose;
            }
            else if(name == "traytoggle") {
                res.action = action_traytoggle;
            }
            else {
                util::warn("invalid option: --" + name);
                res.action = action_help;
                break;
            }
        }
        else if(util::str_startswith(s, "-")) {
            std::string name = s.substr(1);
            if(name == "h") {
                res.action = action_help;
                break;
            }
            else if(name == "d") {
                res.action = action_default;
            }
            else if(name == "t") {
                res.action = action_trayclose;
            }
            else if(name == "T") {
                res.action = action_traytoggle;
            }
            else {
                util::warn("invalid option: -" + name);
                res.action = action_help;
                break;
            }
        }
        else {
            if(!parse_drive(s, res.letter)) {
                util::warn("invalid drive: " + s);
                res.action = action_help;
            }
            break;
        }
    }

    return res;
}

} // anonymous namespace

int main(int argc, char** argv) {
    Args args = parse_args(argc, argv);

    try {
        return args.action(args);
    }
    catch(const std::exception& e) {
        util::error(e.what());
    }
}

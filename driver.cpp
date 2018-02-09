#include <string>

#ifndef STRICT
# define STRICT
#endif
#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
# define NOMINMAX
#endif
#include <windows.h>
#include <winioctl.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#undef STRICT

#include "driver.hpp"
#include "util.hpp"

namespace {

bool is_optical_drive(char letter) {
    std::string path { letter };
    path += ":\\";

    return ::GetDriveType(path.c_str()) == DRIVE_CDROM;
}

class Volume : private util::noncopyable {
public:
    explicit Volume(char letter)
        : handle_(), locked_(false), dismounted_(false), allowed_removal_(false) {
        std::string path = "\\\\.\\" + std::string{letter} + ":";

        handle_ = ::CreateFile(
            /* lpFileName            = */ path.c_str(),
            /* dwDesiredAccess       = */ GENERIC_READ,
            /* dwShareMode           = */ FILE_SHARE_READ | FILE_SHARE_WRITE,
            /* lpSecurityAttributes  = */ nullptr,
            /* dwCreationDisposition = */ OPEN_EXISTING,
            /* dwFlagsAndAttributes  = */ 0,
            /* hTemplateFile         = */ nullptr
        );
        if(handle_ == INVALID_HANDLE_VALUE)
            throw EjectError("CreateFile() failed");
    }

    ~Volume() {
        ::CloseHandle(handle_);
    }

    bool lock() {
        static constexpr int RETRY_COUNT = 3;
        static constexpr int RETRY_WAIT  = 3000; // ms

        if(locked_) return true;

        for(int i = 0; i < RETRY_COUNT; ++i) {
            DWORD dummy;
            BOOL ok = ::DeviceIoControl(
                /* hDevice         = */ handle_,
                /* dwIoControlCode = */ FSCTL_LOCK_VOLUME,
                /* lpInBuffer      = */ nullptr,
                /* nInBufferSize   = */ 0,
                /* lpOutBuffer     = */ nullptr,
                /* nOutBufferSize  = */ 0,
                /* lpBytesReturned = */ &dummy,
                /* lpOverlapped    = */ nullptr
            );
            if(ok) return locked_ = true;

            if(i != RETRY_COUNT-1)
                ::Sleep(RETRY_WAIT);
        }

        return false;
    }

    bool dismount() {
        if(!locked_) return false;
        if(dismounted_) return true;

        DWORD dummy;
        return dismounted_ = ::DeviceIoControl(
            /* hDevice         = */ handle_,
            /* dwIoControlCode = */ FSCTL_DISMOUNT_VOLUME,
            /* lpInBuffer      = */ nullptr,
            /* nInBufferSize   = */ 0,
            /* lpOutBuffer     = */ nullptr,
            /* nOutBufferSize  = */ 0,
            /* lpBytesReturned = */ &dummy,
            /* lpOverlapped    = */ nullptr
        );
    }

    bool allow_removal() {
        if(!locked_ || !dismounted_) return false;
        if(allowed_removal_) return true;

        PREVENT_MEDIA_REMOVAL pmr;
        pmr.PreventMediaRemoval = FALSE;

        DWORD dummy;
        return allowed_removal_ = ::DeviceIoControl(
            /* hDevice         = */ handle_,
            /* dwIoControlCode = */ IOCTL_DISK_MEDIA_REMOVAL,
            /* lpInBuffer      = */ &pmr,
            /* nInBufferSize   = */ sizeof(pmr),
            /* lpOutBuffer     = */ nullptr,
            /* nOutBufferSize  = */ 0,
            /* lpBytesReturned = */ &dummy,
            /* lpOverlapped    = */ nullptr
        );
    }

    bool eject_media() const {
        if(!locked_ || !dismounted_ || !allowed_removal_) return false;

        DWORD dummy;
        return ::DeviceIoControl(
            /* hDevice         = */ handle_,
            /* dwIoControlCode = */ IOCTL_DISK_EJECT_MEDIA,
            /* lpInBuffer      = */ nullptr,
            /* nInBufferSize   = */ 0,
            /* lpOutBuffer     = */ nullptr,
            /* nOutBufferSize  = */ 0,
            /* lpBytesReturned = */ &dummy,
            /* lpOverlapped    = */ nullptr
        );
    }

    bool load_media() const {
        if(!locked_ || !dismounted_ || !allowed_removal_) return false;

        DWORD dummy;
        return ::DeviceIoControl(
            /* hDevice         = */ handle_,
            /* dwIoControlCode = */ IOCTL_DISK_LOAD_MEDIA,
            /* lpInBuffer      = */ nullptr,
            /* nInBufferSize   = */ 0,
            /* lpOutBuffer     = */ nullptr,
            /* nOutBufferSize  = */ 0,
            /* lpBytesReturned = */ &dummy,
            /* lpOverlapped    = */ nullptr
        );
    }

private:
    HANDLE handle_;
    bool locked_;
    bool dismounted_;
    bool allowed_removal_;
};

void tray_prepare(Volume& vol) {
    if(!vol.lock())          throw EjectError("can't lock volume");
    if(!vol.dismount())      throw EjectError("can't dismount volume");
    if(!vol.allow_removal()) throw EjectError("can't allow removal");
}

} // anonymous namespace

char OpticalDrive::find_first() {
    DWORD bits = ::GetLogicalDrives();
    if(!bits) return 0;

    for(int i = 0; i < 26; ++i) {
        if(!(bits & (1<<i))) continue;

        char letter = 'A' + i;
        if(is_optical_drive(letter))
            return letter;
    }

    return 0;
}

OpticalDrive::OpticalDrive(char letter) {
    if(!util::ascii_isalpha(letter))
        throw EjectError("invalid drive letter");

    letter_ = util::ascii_toupper(letter);

    if(!is_optical_drive(letter_))
        throw EjectError("not optical drive: " + std::string{letter_});
}

void OpticalDrive::tray_open() const {
    Volume vol(letter_);

    tray_prepare(vol);

    if(!vol.eject_media()) throw EjectError("can't eject media");
}

void OpticalDrive::tray_close() const {
    Volume vol(letter_);

    tray_prepare(vol);

    if(!vol.load_media()) throw EjectError("can't load media");
}

// The logic is stolen from eject.c in util-linux
void OpticalDrive::tray_toggle() const {
    static constexpr int ALREADY_OPEN_MS = 200;

    Volume vol(letter_);

    tray_prepare(vol);

    DWORD tstart = ::GetTickCount();
    if(!vol.eject_media()) throw EjectError("can't eject media");
    DWORD tend = ::GetTickCount();

    if(tend - tstart < ALREADY_OPEN_MS)
        if(!vol.load_media()) throw EjectError("can't load media");
}

#include "adapt/utils/StringUtils.h"
#include <Windows.h>

namespace mif::ila_lseexport::string_utils {

std::wstring str2wstr(std::string_view str, unsigned int codePage) {
    int          len = MultiByteToWideChar(codePage, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
    std::wstring wstr;
    if (len == 0) return wstr;
    wstr.resize(len);
    MultiByteToWideChar(codePage, 0, str.data(), static_cast<int>(str.size()), wstr.data(), len);
    return wstr;
}

std::string wstr2str(std::wstring_view wstr, unsigned int codePage) {
    int len =
        WideCharToMultiByte(codePage, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
    std::string str;
    if (len == 0) return str;
    str.resize(len);
    WideCharToMultiByte(
        codePage,
        0,
        wstr.data(),
        static_cast<int>(wstr.size()),
        str.data(),
        static_cast<int>(str.size()),
        nullptr,
        nullptr
    );
    return str;
}

std::string str2str(std::string_view str, unsigned int fromCodePage, unsigned int toCodePage) {
    return wstr2str(str2wstr(str, fromCodePage), toCodePage);
}

} // namespace mif::ila_lseexport::string_utils
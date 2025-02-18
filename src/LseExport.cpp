#include "LseExport.h"
#include <ll/api/mod/RegisterHelper.h>

namespace ila {

LseExport& LseExport::getInstance() {
    static LseExport instance;
    return instance;
}

bool LseExport::load() {
    exportEvent();
    return true;
}

bool LseExport::enable() { return true; }

bool LseExport::disable() { return true; }

} // namespace ila

LL_REGISTER_MOD(ila::LseExport, ila::LseExport::getInstance());
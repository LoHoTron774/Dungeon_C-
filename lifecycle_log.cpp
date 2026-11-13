#include "header/lifecycle_log.h"

#include <sstream>
#include <utility>

namespace {
lifecycle::Sink& sink() {
    static lifecycle::Sink s;
    return s;
}
}

namespace lifecycle {

void setSink(Sink s) { sink() = std::move(s); }

void log(const std::string& what, const void* self) {
    if (!sink()) return;
    std::ostringstream os;
    os << "  [" << what << "] this=" << self;
    sink()(os.str());
}

}

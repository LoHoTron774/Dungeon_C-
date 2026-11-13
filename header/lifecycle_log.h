#ifndef LIFECYCLE_LOG_H
#define LIFECYCLE_LOG_H

#include <functional>
#include <string>

namespace lifecycle {

using Sink = std::function<void(const std::string&)>;

void setSink(Sink sink);
void log(const std::string& what, const void* self);

}

#endif

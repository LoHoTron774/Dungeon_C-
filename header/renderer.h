#ifndef RENDERER_H
#define RENDERER_H

#include <iosfwd>
#include <vector>

#include "creature.h"
#include "map.h"

class Renderer {
public:
    void draw(std::ostream& out, const Map& map,
              const std::vector<const Creature*>& monsters, const Creature& hero) const;
};

#endif

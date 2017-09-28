#pragma once

#include "body.h"
#include "Random.h"
#include "primitives.h"
#include "Octree.h"

class CLayout
{
public:
    CLayout();

    bool step();

    void serialize_to_file();

    void serialize_to_file(size_t iteration);

    const std::vector<CBody>& get_bodies() const;

private:
    void load_positions();

    void load_weights();

private:
    void load_links(const fs::path& pathLinksFile);

    void load_positions(const fs::path& pathPositionsFile);

    void load_weights(const fs::path& pathWeightsFile);

private:
    void init_positions();

    void init_weights();

private:
    void accumulate();

    double integrate();

    void updateSpringForce(CBody& source);

    void updateDragForce(CBody& body);
private:
    CRandom m_Random;
    std::vector<CBody> m_vecBodies;
    COctree m_Tree;
};
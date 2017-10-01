#include "stdafx.h"

#include "layout.h"
#include "layout_settings.h"
#include "application_settings.h"

using FileContent = std::vector<int>;
FileContent ReadFile(const fs::path& path)
{
    FileContent content;
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        THROW_EXCEPTION(L"Cant open file = " << path);
    }

    std::streampos size = file.tellg();

    content.resize(size / sizeof(int));
    file.seekg(0, std::ios::beg);
    file.read((char *)content.data(), size);

    return content;
}

CLayout::CLayout()
    : m_Random(42)
{
    load_links(g_Settings.get_links_file());

    load_positions();

    load_weights();
}

void CLayout::serialize_to_file()
{
    std::ofstream outfile(g_Settings.get_save_positions_file(), std::ofstream::binary);

    for (const auto& body : m_vecBodies)
    {
        const auto& pos = body.get_position();
        const int triplet[3]{
            static_cast<int>(floor(pos.x + 0.5)),
            static_cast<int>(floor(pos.y + 0.5)),
            static_cast<int>(floor(pos.z + 0.5))
        };
        outfile.write((const char *)&triplet, sizeof(triplet));
    }

    outfile.flush();
}

void CLayout::serialize_to_file(size_t iteration)
{
    auto path = g_Settings.get_save_positions_file();
    {
        auto filename = path.filename();
        auto stem = filename.stem().string() + "_" + std::to_string(iteration);

        path.replace_filename(fs::path(stem).replace_extension(filename.extension()));
    }

    std::ofstream outfile(path, std::ofstream::binary);

    for (const auto& body : m_vecBodies)
    {
        const auto& pos = body.get_position();
        const int triplet[3]{
            static_cast<int>(floor(pos.x + 0.5)),
            static_cast<int>(floor(pos.y + 0.5)),
            static_cast<int>(floor(pos.z + 0.5))
        };
        outfile.write((const char *)&triplet, sizeof(triplet));
    }

    outfile.flush();
}

const std::vector<CBody>& CLayout::get_bodies() const
{
    return m_vecBodies;
}

void CLayout::load_positions()
{
    if (g_Settings.is_verbose())
    {
        std::wcout << L"Load positions" << std::endl;
    }

    if (g_Settings.is_required_positions_file())
    {
        if (g_Settings.is_verbose())
        {
            std::wcout << L"Load positions from file" << std::endl;
        }

        load_positions(g_Settings.get_positions_file());
    }
    else
    {
        if (g_Settings.is_verbose())
        {
            std::wcout << L"Positions initialized default values" << std::endl;
        }

        init_positions();
    }
}

void CLayout::load_weights()
{
    if (g_Settings.is_verbose())
    {
        std::wcout << L"Load weights" << std::endl;
    }

    if (g_Settings.is_required_weights_file())
    {
        if (g_Settings.is_verbose())
        {
            std::wcout << L"Load weights from file" << std::endl;
        }

        load_weights(g_Settings.get_weights_file());
    }
    else
    {
        if (g_Settings.is_verbose())
        {
            std::wcout << L"Weights initialized default values" << std::endl;
        }

        init_weights();
    }
}

void CLayout::load_links(const fs::path & pathLinksFile)
{
    const auto links = ReadFile(pathLinksFile);
    if (links.empty())
    {
        THROW_EXCEPTION(L"Graph has no links");
    }

    {
        const auto element = std::max_element(
            links.begin(), links.end(),
            [](int a, int b) {
                return std::abs(a) < std::abs(b);
            });

        m_vecBodies.resize(std::abs(*element));
    }

    int from = 0;
    for (int link : links)
    {
        int index = link;
        if (index < 0) {
            index = -index;
            from = index - 1;
        }
        else {
            const int to = index - 1;
            m_vecBodies[from].push_springs(to);
            m_vecBodies[to].inc_in_edges();
        }
    }
}

void CLayout::load_positions(const fs::path & pathPositionsFile)
{
    const auto positions = ReadFile(pathPositionsFile);
    #pragma omp parallel for
    #if _OPENMP >= 200805
    for (size_t i = 0; i < m_vecBodies.size(); ++i)
    #else
    for (int i = 0; i < m_vecBodies.size(); ++i)
    #endif
    {
        m_vecBodies[i].set_position(
            Vector3{
                static_cast<double>(positions[i * 3 + 0]),
                static_cast<double>(positions[i * 3 + 1]),
                static_cast<double>(positions[i * 3 + 2])
            });
    }
}

void CLayout::load_weights(const fs::path & pathWeightsFile)
{
    const auto weights = ReadFile(pathWeightsFile);
    if (weights.empty())
    {
        THROW_EXCEPTION(L"Has no weights");
    }

    if (weights.size() != m_vecBodies.size())
    {
        THROW_EXCEPTION(L"Wrong weights for current graph");
    }

    #pragma omp parallel for
    #if _OPENMP >= 200805
    for (size_t i = 0; i < m_vecBodies.size(); ++i)
    #else
    for (int i = 0; i < m_vecBodies.size(); ++i)
    #endif
    {
        m_vecBodies[i].set_mass(weights[i]);
    }
}

void CLayout::init_positions()
{
    const size_t maxBodyId = m_vecBodies.size();

    for (size_t i = 0; i < m_vecBodies.size(); ++i)
    {
        auto& body = m_vecBodies[i];
        if (!body.get_position().is_initialized())
        {
            Vector3 pos(
                m_Random.nextDouble() * log(maxBodyId) * 100,
                m_Random.nextDouble() * log(maxBodyId) * 100,
                m_Random.nextDouble() * log(maxBodyId) * 100
            );
            body.set_position(pos);
        }
        const Vector3& sourcePos = body.get_position();
        auto& springs = body.get_springs();
        // init neighbours position:
        for (size_t j = 0; j < springs.size(); ++j)
        {
            if (!m_vecBodies[springs[j]].get_position().is_initialized())
            {
                Vector3 pos(
                    sourcePos.x + m_Random.next(LayoutSettings::springLength) - LayoutSettings::springLength / 2,
                    sourcePos.y + m_Random.next(LayoutSettings::springLength) - LayoutSettings::springLength / 2,
                    sourcePos.z + m_Random.next(LayoutSettings::springLength) - LayoutSettings::springLength / 2
                );
                m_vecBodies[springs[j]].set_position(pos);
            }
        }
    }
}

void CLayout::init_weights()
{
    #pragma omp parallel for
    #if _OPENMP >= 200805
    for (size_t i = 0; i < m_vecBodies.size(); ++i)
    #else
    for (int i = 0; i < m_vecBodies.size(); ++i)
    #endif
    {
        auto& body = m_vecBodies[i];
        body.set_mass(1 + (body.get_springs().size() + body.get_incoming_links()) / 3.0);
    }
}

void CLayout::accumulate()
{
    m_Tree.insertBodies(m_vecBodies);

    #pragma omp parallel for
    #if _OPENMP >= 200805
    for (size_t i = 0; i < m_vecBodies.size(); ++i)
    #else
    for (int i = 0; i < m_vecBodies.size(); ++i)
    #endif
    {
        CBody& body = m_vecBodies[i];
        body.reset_force();

        m_Tree.updateBodyForce(body);
        updateDragForce(body);
    }

    #pragma omp parallel for
    #if _OPENMP >= 200805
    for (size_t i = 0; i < m_vecBodies.size(); ++i)
    #else
    for (int i = 0; i < m_vecBodies.size(); ++i)
    #endif
    {
        updateSpringForce(m_vecBodies[i]);
    }
}

double CLayout::integrate()
{
    double dx = 0, tx = 0,
    dy = 0, ty = 0,
    dz = 0, tz = 0;

    //dx should be private or defined inside loop
    //tx need to be reduction variable, or its value will be unpredictable.
    #pragma omp parallel for reduction(+:tx,ty,tz) private(dx,dy,dz)
    #if _OPENMP >= 200805
    for (size_t i = 0; i < m_vecBodies.size(); ++i)
    #else
    for (int i = 0; i < m_vecBodies.size(); ++i)
    #endif
    {
        CBody& body = m_vecBodies[i];
        const double coeff = LayoutSettings::timeStep / body.get_mass();
        const auto& pos = body.get_position();
        const auto& velocity = body.get_velocity();
        const auto& force = body.get_force();

        body.set_velocity(
            Vector3 {
                velocity.x + coeff * force.x,
                velocity.y + coeff * force.y,
                velocity.z + coeff * force.z
            }
        );

        double vx = velocity.x,
            vy = velocity.y,
            vz = velocity.z,
            v = sqrt(vx * vx + vy * vy + vz * vz);

        if (v > 1) {
            body.set_velocity(
                Vector3{ vx / v, vy / v, vz / v }
            );
        }

        dx = LayoutSettings::timeStep * velocity.x;
        dy = LayoutSettings::timeStep * velocity.y;
        dz = LayoutSettings::timeStep * velocity.z;

        body.set_position(
            Vector3 {
                pos.x + dx,
                pos.y + dy,
                pos.z + dz
            }
        );

        tx += abs(dx); ty += abs(dy); tz += abs(dz);
    }

    return (tx * tx + ty * ty + tz * tz)/m_vecBodies.size();
}

bool CLayout::step()
{
    accumulate();

    double totalMovement = integrate();

    if (g_Settings.is_verbose())
    {
        std::wcout << totalMovement << L" move" << std::endl;
    }

    return totalMovement < LayoutSettings::stableThreshold;
}

void CLayout::updateSpringForce(CBody& source) 
{
    const auto& src_pos = source.get_position();
    for(int spring : source.get_springs())
    {
        CBody& body2 = m_vecBodies[spring];
        const auto& dst_pos = body2.get_position();
        double dx = dst_pos.x - src_pos.x;
        double dy = dst_pos.y - src_pos.y;
        double dz = dst_pos.z - src_pos.z;
        double r = sqrt(dx * dx + dy * dy + dz * dz);

        if (r == 0) 
        {
            dx = (m_Random.nextDouble() - 0.5) / 50;
            dy = (m_Random.nextDouble() - 0.5) / 50;
            dz = (m_Random.nextDouble() - 0.5) / 50;
            r = sqrt(dx * dx + dy * dy + dz * dz);
        }

        const double d = r - LayoutSettings::springLength;
        const double coeff = LayoutSettings::springCoeff * d / r;

        {
            const auto& force = source.get_force();
            source.set_force(
                Vector3 {
                    force.x + coeff * dx,
                    force.y + coeff * dy,
                    force.z + coeff * dz
                }
            );
        }

        {
            const auto& force = body2.get_force();
            body2.set_force(
                Vector3 {
                    force.x - coeff * dx,
                    force.y - coeff * dy,
                    force.z - coeff * dz
                }
            );
        }
    }
}

void CLayout::updateDragForce(CBody& body)
{
    const auto& force = body.get_force();
    const auto& velocity = body.get_velocity();
    body.set_force(
        Vector3 {
            force.x - LayoutSettings::dragCoeff * velocity.x,
            force.y - LayoutSettings::dragCoeff * velocity.y,
            force.z - LayoutSettings::dragCoeff * velocity.z
        }
    );
}

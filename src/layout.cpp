#include <stdafx.h>

#include <layout.h>
#include <layout_settings.h>
#include <application_settings.h>

using FileContent = std::vector<int>;
FileContent ReadFile(const fs::path& path)
{
    FileContent content;
    std::ifstream file(path.string(), std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        THROW_EXCEPTION("Cant open file = " << path);
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
    std::ofstream outfile(g_Settings.get_save_positions_file().string(), std::ofstream::binary);

    for (auto& body : m_vecBodies)
    {
        auto& pos = body.get_position();
        const int triplet[coord::num]{
            static_cast<int>(floor(pos[X] + 0.5)),
            static_cast<int>(floor(pos[Y] + 0.5)),
            static_cast<int>(floor(pos[Z] + 0.5))
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

        path = filename.parent_path() / fs::path(stem).replace_extension(filename.extension());
    }

    std::ofstream outfile(path.string(), std::ofstream::binary);

    for (auto& body : m_vecBodies)
    {
        auto& pos = body.get_position();
        const int triplet[coord::num]{
            static_cast<int>(floor(pos[X] + 0.5)),
            static_cast<int>(floor(pos[Y] + 0.5)),
            static_cast<int>(floor(pos[Z] + 0.5))
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
        std::cout << "Load positions" << std::endl;
    }

    if (g_Settings.is_required_positions_file())
    {
        if (g_Settings.is_verbose())
        {
            std::cout << "Load positions from file" << std::endl;
        }

        load_positions(g_Settings.get_positions_file());
    }
    else
    {
        if (g_Settings.is_verbose())
        {
            std::cout << "Positions initialized default values" << std::endl;
        }

        init_positions();
    }
}

void CLayout::load_weights()
{
    if (g_Settings.is_verbose())
    {
        std::cout << "Load weights" << std::endl;
    }

    if (g_Settings.is_required_weights_file())
    {
        if (g_Settings.is_verbose())
        {
            std::cout << "Load weights from file" << std::endl;
        }

        load_weights(g_Settings.get_weights_file());
    }
    else
    {
        if (g_Settings.is_verbose())
        {
            std::cout << "Weights initialized default values" << std::endl;
        }

        init_weights();
    }
}

void CLayout::load_links(const fs::path & pathLinksFile)
{
    const auto links = ReadFile(pathLinksFile);
    if (links.empty())
    {
        THROW_EXCEPTION("Graph has no links");
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
    for (int64_t i = 0; i < m_vecBodies.size(); ++i)
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
        THROW_EXCEPTION("Has no weights");
    }

    if (weights.size() != m_vecBodies.size())
    {
        THROW_EXCEPTION("Wrong weights for current graph");
    }

    #pragma omp parallel for
    #if _OPENMP >= 200805
    for (size_t i = 0; i < m_vecBodies.size(); ++i)
    #else
    for (int64_t i = 0; i < m_vecBodies.size(); ++i)
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
            const Vector3 pos = Vector3{ m_Random.nextDouble(),m_Random.nextDouble(), m_Random.nextDouble() };
            body.set_position(pos * (log(maxBodyId) * 100.0));
        }
        const Vector3& sourcePos = body.get_position();
        // init neighbours position:
        for (int spring : body.get_springs())
        {
            if (!m_vecBodies[spring].get_position().is_initialized())
            {
                const Vector3 add_coeff{
                    m_Random.next(LayoutSettings::springLength) - LayoutSettings::springLength / 2,
                    m_Random.next(LayoutSettings::springLength) - LayoutSettings::springLength / 2,
                    m_Random.next(LayoutSettings::springLength) - LayoutSettings::springLength / 2
                };

                m_vecBodies[spring].set_position(sourcePos + add_coeff);
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
    for (int64_t i = 0; i < m_vecBodies.size(); ++i)
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
    for (int64_t i = 0; i < m_vecBodies.size(); ++i)
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
    for (int64_t i = 0; i < m_vecBodies.size(); ++i)
    #endif
    {
        updateSpringForce(m_vecBodies[i]);
    }
}

double CLayout::integrate()
{
    std::vector<Vector3> vecT(m_vecBodies.size());

    #pragma omp parallel for
    #if _OPENMP >= 200805
    for (size_t i = 0; i < m_vecBodies.size(); ++i)
    #else
    for (int64_t i = 0; i < m_vecBodies.size(); ++i)
    #endif
    {
        CBody& body = m_vecBodies[i];
        const double coeff_v = LayoutSettings::timeStep / body.get_mass();

        const auto& pos = body.get_position();
        const auto& velocity = body.get_velocity();
        const auto& force = body.get_force();

        body.set_velocity(velocity + force * coeff_v);

        const double v = sqrt((velocity * velocity).summ_elem());

        if (v > 1) 
        {
            body.set_velocity(velocity / v);
        }

        const Vector3 diff = velocity * LayoutSettings::timeStep;

        body.set_position(pos + diff);

        vecT[i] = Vector3{
            std::abs(diff[X]),
            std::abs(diff[Y]),
            std::abs(diff[Z])
        };
    }

    Vector3 t;
    for (const auto& v : vecT)
        t += v;

    return (t * t).summ_elem()/m_vecBodies.size();
}

bool CLayout::step()
{
    accumulate();

    double totalMovement = integrate();

    if (g_Settings.is_verbose())
    {
        std::cout << totalMovement << " move" << std::endl;
    }

    return totalMovement < LayoutSettings::stableThreshold;
}

void CLayout::updateSpringForce(CBody& source) 
{
    const auto& src_pos = source.get_position();
    const auto& src_force = source.get_force();
    for(int spring : source.get_springs())
    {
        CBody& body2 = m_vecBodies[spring];
        const auto& dst_pos = body2.get_position();
        Vector3 diff = dst_pos - src_pos;
        double r = sqrt((diff * diff).summ_elem());

        if (r == 0) 
        {
            diff = Vector3{ m_Random.nextDouble(),m_Random.nextDouble(), m_Random.nextDouble() };
            diff = (diff - Vector3{ 0.5 }) / Vector3{ 50.0 };

            r = sqrt((diff * diff).summ_elem());
        }

        const double d = r - LayoutSettings::springLength;
        const double coeff_v = LayoutSettings::springCoeff * d / r;

        source.set_force(src_force + diff * coeff_v);
        body2.set_force(body2.get_force() - diff * coeff_v);
    }
}

void CLayout::updateDragForce(CBody& body)
{
    const auto& force = body.get_force();
    const auto& velocity = body.get_velocity();
    body.set_force(force - velocity * LayoutSettings::dragCoeff);
}

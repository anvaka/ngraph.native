#pragma once

class CApplicationSettings
{
public:
    inline bool is_verbose() const;

    inline size_t get_max_iteration() const;

    inline size_t get_interval_save() const;

    inline bool is_required_links_file() const;

    inline bool is_required_positions_file() const;

    inline bool is_required_weights_file() const;

    inline const fs::path& get_save_positions_file() const;

    inline const fs::path& get_links_file() const;

    inline const fs::path& get_positions_file() const;

    inline const fs::path& get_weights_file() const;

public:
    inline void set_verbose(bool bVerbose);

    inline void set_max_iteration(size_t nMaxIteration);

    inline void set_interval_save(size_t nIntervalSave);

    inline void set_save_positions_file(const fs::path& pathSavePositions);

    inline void set_links_file(const fs::path& pathLinksFile);

    inline void set_positions_file(const fs::path& pathPositionsFile);

    inline void set_weights_file(const fs::path& pathWeightsFile);

private:
    bool m_bVerbose = false;

private:
    fs::path m_pathSavePositions = "./positions.bin";

private:
    bool m_bLinksFileRequired = true;
    fs::path m_pathLinksFile = "./links.bin";

    bool m_bPositionsFileRequired = false;
    fs::path m_pathPositionsFile = "./positions.bin";

    bool m_bWeightsFileRequired = false;
    fs::path m_pathWeightsFile = "./weights.bin";

private:
    size_t m_nMaxIteration = 10000;
    size_t m_nIntervalSave = 10;
};

extern CApplicationSettings g_Settings;

inline void CApplicationSettings::set_verbose(bool bVerbose)
{
    m_bVerbose = bVerbose;
}

inline bool CApplicationSettings::is_verbose() const
{
    return m_bVerbose;
}

inline void CApplicationSettings::set_save_positions_file(const fs::path & pathSavePositions)
{
    m_pathSavePositions = pathSavePositions;
}

inline const fs::path & CApplicationSettings::get_save_positions_file() const
{
    return m_pathSavePositions;
}

inline void CApplicationSettings::set_links_file(const fs::path & pathLinksFile)
{
    m_bLinksFileRequired = true;
    m_pathLinksFile = pathLinksFile;
}

inline const fs::path & CApplicationSettings::get_links_file() const
{
    return m_pathLinksFile;
}

inline void CApplicationSettings::set_positions_file(const fs::path & pathPositionsFile)
{
    m_bPositionsFileRequired = true;
    m_pathPositionsFile = pathPositionsFile;
}

inline const fs::path & CApplicationSettings::get_positions_file() const
{
    return m_pathPositionsFile;
}

inline void CApplicationSettings::set_weights_file(const fs::path & pathWeightsFile)
{
    m_bWeightsFileRequired = true;
    m_pathWeightsFile = pathWeightsFile;
}

inline const fs::path & CApplicationSettings::get_weights_file() const
{
    return m_pathWeightsFile;
}

inline void CApplicationSettings::set_max_iteration(size_t nMaxIteration)
{
    m_nMaxIteration = nMaxIteration;
}

inline size_t CApplicationSettings::get_max_iteration() const
{
    return m_nMaxIteration;
}

inline void CApplicationSettings::set_interval_save(size_t nIntervalSave)
{
    m_nIntervalSave = nIntervalSave;
}

inline size_t CApplicationSettings::get_interval_save() const
{
    return m_nIntervalSave;
}

inline bool CApplicationSettings::is_required_links_file() const
{
    return m_bLinksFileRequired;
}

inline bool CApplicationSettings::is_required_positions_file() const
{
    return m_bPositionsFileRequired;
}

inline bool CApplicationSettings::is_required_weights_file() const
{
    return m_bWeightsFileRequired;
}

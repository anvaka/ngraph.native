#pragma once

class CApplicationSettings
{
public:
    bool is_verbose() const;

    size_t get_max_iteration() const;

    size_t get_interval_save() const;

    bool is_required_links_file() const;

    bool is_required_positions_file() const;

    bool is_required_weights_file() const;

    const fs::path& get_save_positions_file() const;

    const fs::path& get_links_file() const;

    const fs::path& get_positions_file() const;

    const fs::path& get_weights_file() const;

public:
    void set_verbose(bool bVerbose);

    void set_max_iteration(size_t nMaxIteration);

    void set_interval_save(size_t nIntervalSave);

    void set_save_positions_file(const fs::path& pathSavePositions);

    void set_links_file(const fs::path& pathLinksFile);

    void set_positions_file(const fs::path& pathPositionsFile);

    void set_weights_file(const fs::path& pathWeightsFile);

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

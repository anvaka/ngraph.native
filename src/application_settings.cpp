#include "stdafx.h"

#include "application_settings.h"

CApplicationSettings g_Settings;

void CApplicationSettings::set_verbose(bool bVerbose)
{
    m_bVerbose = bVerbose;
}

bool CApplicationSettings::is_verbose() const
{
    return m_bVerbose;
}

void CApplicationSettings::set_save_positions_file(const fs::path & pathSavePositions)
{
    m_pathSavePositions = pathSavePositions;
}

const fs::path & CApplicationSettings::get_save_positions_file() const
{
    return m_pathSavePositions;
}

void CApplicationSettings::set_links_file(const fs::path & pathLinksFile)
{
    m_bLinksFileRequired = true;
    m_pathLinksFile = pathLinksFile;
}

const fs::path & CApplicationSettings::get_links_file() const
{
    return m_pathLinksFile;
}

void CApplicationSettings::set_positions_file(const fs::path & pathPositionsFile)
{
    m_bPositionsFileRequired = true;
    m_pathPositionsFile = pathPositionsFile;
}

const fs::path & CApplicationSettings::get_positions_file() const
{
    return m_pathPositionsFile;
}

void CApplicationSettings::set_weights_file(const fs::path & pathWeightsFile)
{
    m_bWeightsFileRequired = true;
    m_pathWeightsFile = pathWeightsFile;
}

const fs::path & CApplicationSettings::get_weights_file() const
{
    return m_pathWeightsFile;
}

void CApplicationSettings::set_max_iteration(size_t nMaxIteration)
{
    m_nMaxIteration = nMaxIteration;
}

size_t CApplicationSettings::get_max_iteration() const
{
    return m_nMaxIteration;
}

void CApplicationSettings::set_interval_save(size_t nIntervalSave)
{
    m_nIntervalSave = nIntervalSave;
}

size_t CApplicationSettings::get_interval_save() const
{
    return m_nIntervalSave;
}

bool CApplicationSettings::is_required_links_file() const 
{
    return m_bLinksFileRequired;
}

bool CApplicationSettings::is_required_positions_file() const 
{ 
    return m_bPositionsFileRequired;
}

bool CApplicationSettings::is_required_weights_file() const 
{
    return m_bWeightsFileRequired;
}

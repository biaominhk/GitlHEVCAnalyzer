#include "filterloader.h"
#include "exceptions/invaildfilterindexexception.h"
#include "model/modellocator.h"
#include "model/common/comrom.h"
#include <QDir>
#include <QDebug>

#define PLUGIN_DIRECTORY "plugins" ///< plugin directory

FilterLoader::FilterLoader()
{
    this->m_strPluginDir = PLUGIN_DIRECTORY;
}

FilterLoader::~FilterLoader()
{
}

bool FilterLoader::initAllFilters()
{
    xPrepareFilterContext();
    for(int i = 0; i < m_apcFilters.size(); i++)
        if(m_apcFilters[i]->init(&m_cFilterContext) == false )
            qWarning() << QString("Plugin Filter %1 Init Failed!").arg(m_apcFilters[i]->getName());
    return true;
}

bool FilterLoader::uninitAllFilters()
{
    xPrepareFilterContext();
    for(int i = 0; i < m_apcFilters.size(); i++)
        if(m_apcFilters[i]->uninit(&m_cFilterContext) == false )
            qWarning() << QString("Plugin Filter %1 Uninit Failed!").arg(m_apcFilters[i]->getName());
    return true;
}




bool FilterLoader::reinitAllFilters()
{
    uninitAllFilters();
    initAllFilters();
    return true;
}

bool FilterLoader::reloadAllFilters()
{
    unloadAllFilters();

    QDir cPluginsDir(m_strPluginDir);
    QStringList cFilters;
    cFilters << "*.dll" << "*.so" << "*.dylib";
    /// try to load each file as plugin in the directory
    foreach(QString strPluginFileName, cPluginsDir.entryList(cFilters, QDir::Files))
    {
        QPluginLoader* pLoader = new QPluginLoader(cPluginsDir.absoluteFilePath(strPluginFileName));
        AbstractFilter *pPlugin =  qobject_cast<AbstractFilter*>(pLoader->instance());
        ///  loading success or fail
        if (pPlugin)
        {            
            ///< ensure filter name is unique
            if( getFilterByName(pPlugin->getName()) == NULL )
            {
                m_apcPluginLoaders.push_back(pLoader);
                m_apcFilters.push_back(pPlugin);
                qDebug() << QString("Plugin Filter %1 Loading Succeeded!").arg(strPluginFileName);
            }
            else
            {
                qWarning() << QString("Filter Name '%1' Already Existed!").arg(pPlugin->getName());
                delete pPlugin;
                delete pLoader;
            }
        }
        else
        {
            qWarning() << QString("Plugin Filter %1 Loading Failed!").arg(strPluginFileName);
            delete pLoader;
        }
    }

    /// sort
    xReadAndSortFilters();

    /// init
    initAllFilters();
    return true;
}


bool FilterLoader::unloadAllFilters()
{
    /// uninit each filter
    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        m_apcFilters[i]->uninit(&m_cFilterContext);
    }


    /// traverse the list and free all
    while( !m_apcFilters.empty() )
    {
        AbstractFilter* pFilter = m_apcFilters.back();
        QString strFilterName = pFilter->getName();
        delete pFilter;
        m_apcFilters.pop_back();

    }

    while( !m_apcPluginLoaders.empty() )
    {
        QPluginLoader* pLoader = m_apcPluginLoaders.back();
        delete pLoader;
        m_apcPluginLoaders.pop_back();
    }

    return true;
}

bool FilterLoader::config(int iFilterIndex)
{

    if(iFilterIndex >= m_apcFilters.size() || iFilterIndex < 0)
        throw InvaildFilterIndexException();

    // prepare filter context
    xPrepareFilterContext();


    // config filter
    m_apcFilters[iFilterIndex]->config(&m_cFilterContext);
    return true;
}

bool FilterLoader::config(AbstractFilter* pcFilter)
{
    if(pcFilter == NULL)
        throw InvaildFilterIndexException();

    // prepare filter context
    xPrepareFilterContext();

    // config filter
    pcFilter->config(&m_cFilterContext);
    return true;
}

bool FilterLoader::drawTU   (QPainter* pcPainter, ComTU *pcTU,       double dScale,  QRect* pcScaledArea)
{
    // prepare filter context
    xPrepareFilterContext();

    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->drawTU(&m_cFilterContext, pcPainter, pcTU, dScale, pcScaledArea);
        }
    }
    return true;
}

bool FilterLoader::drawPU  (QPainter* pcPainter, ComPU *pcPU,  double dScale, QRect* pcScaledArea)
{
    // prepare filter context
    xPrepareFilterContext();

    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->drawPU(&m_cFilterContext, pcPainter, pcPU, dScale, pcScaledArea);
        }
    }
    return true;
}

bool FilterLoader::drawCU  (QPainter* pcPainter, ComCU *pcCU, double dScale, QRect* pcScaledArea)
{
    // prepare filter context
    xPrepareFilterContext();

    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->drawCU(&m_cFilterContext, pcPainter, pcCU, dScale, pcScaledArea);
        }
    }


    return true;
}

bool FilterLoader::drawCTU  (QPainter* pcPainter, ComCU *pcCU, double dScale, QRect* pcScaledArea)
{
    // prepare filter context
    xPrepareFilterContext();

    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->drawCTU(&m_cFilterContext, pcPainter, pcCU, dScale, pcScaledArea);
        }
    }
    return true;
}

bool FilterLoader::drawTile(QPainter* pcPainter, ComTile *pcTile, double dScale, QRect* pcScaledArea)
{
    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->drawTile(&m_cFilterContext, pcPainter, pcTile, dScale, pcScaledArea);
        }
    }
    return true;
}


bool FilterLoader::drawFrame(QPainter* pcPainter, ComFrame *pcFrame, double dScale, QRect* pcScaledArea)
{
    // prepare filter context
    xPrepareFilterContext();

    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->drawFrame(&m_cFilterContext, pcPainter, pcFrame, dScale, pcScaledArea);
        }
    }
    return true;
}

bool FilterLoader::mousePress(QPainter *pcPainter, ComFrame *pcFrame,
                              const QPointF *pcUnscaledPos, const QPointF *scaledPos,
                              double dScale, Qt::MouseButton eMouseBtn)
{
    // prepare filter context
    xPrepareFilterContext();

    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->mousePress(&m_cFilterContext, pcPainter, pcFrame, pcUnscaledPos, scaledPos, dScale, eMouseBtn);
        }
    }
    return true;
}

bool FilterLoader::keyPress(QPainter *pcPainter, ComFrame *pcFrame, int iKeyPressed)
{
    // prepare filter context
    xPrepareFilterContext();

    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->keyPress(&m_cFilterContext, pcPainter, pcFrame, iKeyPressed);
        }
    }
    return true;
}


AbstractFilter* FilterLoader::getFilterByName(const QString &strFilterName)
{
    AbstractFilter* pFilter = NULL;
    for(int i = 0; i < m_apcFilters.size(); i++)
    {        
        if( m_apcFilters[i]->getName() == strFilterName )
        {
            pFilter = m_apcFilters[i];
            break;
        }
    }
    return pFilter;
}


void FilterLoader::xReadAndSortFilters()
{
    ///read filter order
    QStringList cLastOrder;
    g_cAppSetting.beginGroup("Filters");
    int iSize = g_cAppSetting.beginReadArray("filter_order");
    for (int i = 0; i < iSize; ++i)
    {
        g_cAppSetting.setArrayIndex(i);
        const QString& strFilterName = g_cAppSetting.value("filter_name").toString();
        cLastOrder.push_back(strFilterName);
    }
    g_cAppSetting.endArray();
    g_cAppSetting.endGroup();

    /// sorting
    sortFilters(cLastOrder);
}

void FilterLoader::sortFilters(const QStringList &rcFilterNames)
{
    QStringList cFilterNamesCopy = rcFilterNames;

    for (int i = 0; i < cFilterNamesCopy.size(); ++i)
    {
        bool bFound = false;
        for(int j = 0; j < m_apcFilters.size(); j++)
        {
            if(m_apcFilters.at(j)->getName() == cFilterNamesCopy.at(i))
            {
                bFound = true;
                /// swap position i & j
                AbstractFilter* tempFilter = m_apcFilters.at(i);
                m_apcFilters.replace(i, m_apcFilters.at(j));
                m_apcFilters.replace(j, tempFilter);

                QPluginLoader* tempLoader = m_apcPluginLoaders.at(i);
                m_apcPluginLoaders.replace(i, m_apcPluginLoaders.at(j));
                m_apcPluginLoaders.replace(j, tempLoader);
            }

        }
        if(bFound == false) /// earse the filters that are missing
        {
            cFilterNamesCopy.erase(cFilterNamesCopy.begin()+i);
            i--;
        }
    }
}

void FilterLoader::xPrepareFilterContext()
{
    /// prepare filter context
    ModelLocator* pModel = ModelLocator::getInstance();
    m_cFilterContext.pcBuffer = &pModel->getFrameBuffer();
    m_cFilterContext.pcDrawEngine = &pModel->getDrawEngine();
    m_cFilterContext.pcFilterLoader = &pModel->getDrawEngine().getFilterLoader();
    m_cFilterContext.pcSequenceManager = &pModel->getSequenceManager();
    m_cFilterContext.pcSelectionManager = &pModel->getSelectionManager();
}


void FilterLoader::saveFilterOrder()
{
    ///save filter order
    g_cAppSetting.beginGroup("Filters");
    g_cAppSetting.beginWriteArray("filter_order");
    for (int i = 0; i < m_apcFilters.size(); ++i)
    {
        g_cAppSetting.setArrayIndex(i);
        g_cAppSetting.setValue("filter_name", m_apcFilters.at(i)->getName());
    }
    g_cAppSetting.endArray();
    g_cAppSetting.endGroup();
    g_cAppSetting.sync();
}

QStringList FilterLoader::getFilterNames()
{
    QStringList cNameList;
    foreach(AbstractFilter* pcFilter, m_apcFilters )
        cNameList << pcFilter->getName();
    return cNameList;
}

QVector<bool> FilterLoader::getEnableStatus()
{
    QVector<bool> cEnableStatus;
    foreach(AbstractFilter* pcFilter, m_apcFilters )
        cEnableStatus.push_back(pcFilter->getEnable());
    return cEnableStatus;
}

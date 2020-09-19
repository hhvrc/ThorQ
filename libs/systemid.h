#ifndef SYSTEMID_H
#define SYSTEMID_H

#include <QString>
#include <QByteArray>

namespace ThorQ {
namespace SystemID_Internal {

/**
 * @brief getMacHash
 * @param mac1
 * @param mac2
 */
void getMacHash(quint16& mac1, quint16& mac2);

/**
 * @brief getVolumeHash
 * @return
 */
quint16 getVolumeHash();

/**
 * @brief getCpuHash
 * @return
 */
quint16 getCpuHash();

/**
 * @brief getMachineName
 * @return
 */
const char* getMachineName();
}

/**
 * @brief systemid_generate
 * @return
 */
QByteArray systemid_generate();

/**
 * @brief systemid_validate
 * @param sys_id
 * @return
 */
bool systemid_validate(const QByteArray& sys_id);

/**
 * @brief systemid_to_string
 * @param sys_id
 * @return
 */
QString systemid_to_string(QByteArray sys_id);
}

#endif // SYSTEMID_H

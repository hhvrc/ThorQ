
 * @brief thorq_payload_login_pack
 * @param payload
 * @param cmd_id
 */
inline void thorq_payload_login_pack(std::vector<std::uint8_t>& payload, const QString& username, const QString& password)
{
    QStringRef usernameRef(&username);
    usernameRef.truncate(THORQ_USERNAME_LEN_MAX);
    QByteArray usernameBytes = usernameRef.toUtf8();

    QStringRef passwordRef(&password);
    passwordRef.truncate(THORQ_PASSWORD_LEN_MAX);
    QByteArray passwordBytes = passwordRef.toUtf8();

    payload.resize(3 + usernameBytes.size() + passwordBytes.size());
    payload[0] = THORQ_PAYLOAD_ID_LOGIN;
    payload[1] = usernameBytes.size();
    payload[2] = passwordBytes.size();

    memcpy(payload.data() + 3, usernameBytes.data(), usernameBytes.size());
    memcpy(payload.data() + 3 + usernameBytes.size(), passwordBytes.data(), passwordBytes.size());
}

/**
 * @brief thorq_payload_login_get_username
 * @param payload
 * @param username
 */
inline void thorq_payload_login_get_username(const std::vector<std::uint8_t>& payload, QString& username)
{
    username.fromUtf8((const char*)payload.data() + 3, payload[1]);
}

/**
 * @brief thorq_payload_login_get_password
 * @param payload
 * @param password
 */
inline void thorq_payload_login_get_password(const std::vector<std::uint8_t>& payload, QString& password)
{
    password.fromUtf8((const char*)payload.data() + 3 + payload[1], payload[2]);
}

#endif // THORQ_PAYLOAD_LOGIN_H

#ifndef ATHOMENETWORKMODULE_HPP
# define ATHOMENETWORKMODULE_HPP
# include "AtHomeConfig.h"
# if !defined(DISABLE_COMMUNICATION) && !defined(DISABLE_NETWORK)
#  include <stdio.h>
#  include "AtHomeModule.hpp"
#  include "AtHomeFlashCommon.h"

namespace athome {
    namespace module {
        template <typename T, size_t n>
        class AtHomeNetworkModule : public AtHomeModule<T, n> {
        friend class AtHomeModule<T, n>;
        public:
            AtHomeNetworkModule(const AtHomeNetworkModule &) = delete;
            AtHomeNetworkModule &operator=(const AtHomeNetworkModule &) = delete;
            ~AtHomeNetworkModule() {}

            virtual void setCommandPlugin(typename AtHomeModule<T, n>::AtHomeCommandPlugin plugin) {
                _networkCommandCallback = plugin;
            }

            virtual void setOnBackupPlugin(typename AtHomeModule<T, n>::AtHomeStoragePlugin plugin) {
                _networkOnBackupCallback = plugin;
            }

            virtual void setOnRestorePlugin(typename AtHomeModule<T, n>::AtHomeStoragePlugin plugin) {
                _networkOnRestoreCallback = plugin;
            }

            void setNetworkCommunicator(communication::ANetworkCommunicator &communicator) {
                if (_communicator != nullptr) {
                    _communicator->disconnectFromHost();
                }
                _communicator = &communicator;
                this->onRestoreFromStorage();
            }

            void setHost(const communication::ip::tcp_host &host) {
                if (_communicator != nullptr) {
                    _communicator->disconnectFromHost();
                }
                _communicator->setHost(host);
                this->onBackupOnStorage();
            }

        protected:
            AtHomeNetworkModule():
                    _networkCommandCallback(nullptr),
                    _networkOnBackupCallback(nullptr),
                    _networkOnRestoreCallback(nullptr),
                    _communicator(nullptr),
                    AtHomeModule<T, n>() {
                AtHomeModule<T, n>::setCommandPlugin(&AtHomeNetworkModule::_onCommandReceivedNetwork);
                AtHomeModule<T, n>::setOnBackupPlugin(&AtHomeNetworkModule::_onNetworkBackup);
                AtHomeModule<T, n>::setOnRestorePlugin(&AtHomeNetworkModule::_onNetworkRestore);
            }

            void onNetworkBackup(size_t offset, storage::IStorage &storage) {
                if (_communicator == nullptr) {
                    if (_networkOnBackupCallback != nullptr) {
                        _networkOnBackupCallback(offset, storage);
                    }
                    return;
                }
                communication::ip::tcp_host host = _communicator->getHost();
                storage.write(offset, reinterpret_cast<const void *>(host.ipv6), sizeof(host.ipv6));
                offset += sizeof(host.ipv6);
                storage.write(offset, reinterpret_cast<const void *>(&(host.hport)), sizeof(host.hport));
                offset += sizeof(host.hport);
                if (_networkOnBackupCallback != nullptr) {
                    _networkOnBackupCallback(offset, storage);
                }
            }

            void onNetworkRestore(size_t offset, storage::IStorage &storage) {
                if (_communicator == nullptr) {
                    if (_networkOnRestoreCallback != nullptr) {
                        _networkOnRestoreCallback(offset, storage);
                    }
                    return;
                }
                communication::ip::tcp_host host;
                storage.read(offset, reinterpret_cast<void *>(host.ipv6), sizeof(host.ipv6));
                offset += sizeof(host.ipv6);
                storage.read(offset, reinterpret_cast<void *>(&(host.hport)), sizeof(host.hport));
                offset += sizeof(host.hport);
                _communicator->setHost(host);
                if (_networkOnRestoreCallback != nullptr) {
                    _networkOnRestoreCallback(offset, storage);
                }
            }

        private:
            void setEndPointCommand(Stream &communicator) {
                communication::ip::tcp_host host;
                int version = communicator.read();
                if (version == 4) {
                    for (uint8_t i = 0; i < 4; i++) {
                        host.ipv4[i] = communicator.read();
                    }
                }
                else if (version == 6) {
                    for (uint8_t i = 0; i < 16; i++) {
                        host.ipv6[i] = communicator.read();
                    }
                }
                else {
                    while (communicator.read() != communication::commands::end_of_command);
                    return;
                }
                host.hport = communicator.read();
                host.hport |= (communicator.read() << 8);
                while (communicator.read() != communication::commands::end_of_command);
                if (_communicator != nullptr) {
                    _communicator->setHost(host);
                    this->onBackupOnStorage();
                }
            }

        private:
            static void _onCommandReceivedNetwork(const String &command, Stream &stream) {
                AtHomeNetworkModule *instance = reinterpret_cast<AtHomeNetworkModule *>(AtHomeModule<T, n>::getInstance());
                if (instance == nullptr) {
                    return;
                }
                if (!STRCMP(command.c_str(), communication::commands::setEndPoint)) {
                    instance->setEndPointCommand(stream);
                }
                else if (instance->_networkCommandCallback != nullptr) {
                    instance->_networkCommandCallback(command, stream);
                }
                else {
                    while (stream.read() != communication::commands::end_of_command);
                }
            }

            static void _onNetworkBackup(size_t offset, storage::IStorage &storage) {
                AtHomeNetworkModule *instance = reinterpret_cast<AtHomeNetworkModule *>(AtHomeModule<T, n>::getInstance());
                if (instance != nullptr) {
                    instance->onNetworkBackup(offset, storage);
                }
            }

            static void _onNetworkRestore(size_t offset, storage::IStorage &storage) {
                AtHomeNetworkModule *instance = reinterpret_cast<AtHomeNetworkModule *>(AtHomeModule<T, n>::getInstance());
                if (instance != nullptr) {
                    instance->onNetworkRestore(offset, storage);
                }
            }

        private:
            typename AtHomeModule<T, n>::AtHomeCommandPlugin _networkCommandCallback;
            typename AtHomeModule<T, n>::AtHomeStoragePlugin _networkOnBackupCallback;
            typename AtHomeModule<T, n>::AtHomeStoragePlugin _networkOnRestoreCallback;

        protected:
            communication::ANetworkCommunicator             *_communicator;
        };
    }
}

# endif /* !defined(DISABLE_COMMUNICATION) && !defined(DISABLE_NETWORK) */
#endif /* ATHOMENETWORKMODULE_HPP */
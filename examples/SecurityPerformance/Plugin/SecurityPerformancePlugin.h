/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 Metrological
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#pragma once

#include "Module.h"

#include <websocket/websocket.h>
#include <interfaces/ISecurityPerformance.h>

namespace Thunder {

namespace Plugin {

    class SecurityPerformancePlugin : public PluginHost::IPlugin, public Exchange::ISecurityPerformance {
    private:
        // We do not allow this plugin to be copied !!
        SecurityPerformancePlugin(const SecurityPerformancePlugin&) = delete;
        SecurityPerformancePlugin& operator=(const SecurityPerformancePlugin&) = delete;

        // The next class describes configuration information for this plugin.
        class Config : public Core::JSON::Container {
        private:
            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

        public:
            Config()
                : Core::JSON::Container()
                , Connector("0.0.0.0:8899")
            {
                Add(_T("connector"), &Connector);
            }
            ~Config() = default;

        public:
            Core::JSON::String Connector;
        };

        class COMServer : public RPC::Communicator {
        public:
            COMServer() = delete;
            COMServer(const COMServer&) = delete;
            COMServer& operator=(const COMServer&) = delete;

        public:
            COMServer(
                const Core::NodeId& source, 
                Exchange::ISecurityPerformance* parentInterface, 
                const string& proxyStubPath, 
                const Core::ProxyType<RPC::InvokeServer>& engine)
                : RPC::Communicator(source, proxyStubPath, Core::ProxyType<Core::IIPCServer>(engine))
                , _parentInterface(parentInterface)
            {
                Open(Core::infinite);
            }
            ~COMServer()
            {
                Close(Core::infinite);
            }

        private:
            virtual void* Acquire(const string& className VARIABLE_IS_NOT_USED, const uint32_t interfaceId, const uint32_t versionId)
            {
                void* result = nullptr;

                // Currently we only support version 1 of the IRPCLink :-)
                if ((versionId == 1) || (versionId == static_cast<uint32_t>(~0))) {
                    // Reference count our parent
                    result = _parentInterface->QueryInterface(interfaceId);

                    printf("Pointer => %p\n", result);
                }
                return (result);
            }

        private:
            Exchange::ISecurityPerformance* _parentInterface;
        };

        class Forwarder : public ISecurityPerformance::IForwarder {
        public:
            Forwarder(const Forwarder&&) = delete;
            Forwarder& operator=(const Forwarder&) = delete;

            Forwarder() {}
            ~Forwarder() = default;

            BEGIN_INTERFACE_MAP(Forwarder)
            INTERFACE_ENTRY(Exchange::ISecurityPerformance::IForwarder)
            END_INTERFACE_MAP

        public:
            Core::hresult Sum(const uint32_t a, const uint32_t b, uint32_t& sum) const override;
            uint32_t AddRef() const override
            {
                Core::InterlockedIncrement(_refCount);
                return Core::ERROR_NONE;
            }
            uint32_t Release() const override
            {
                if (Core::InterlockedDecrement(_refCount) == 0) {
                    delete this;
                }
                return (Core::ERROR_NONE);
            }
        private:
            mutable uint32_t _refCount;
        };

    public:
        SecurityPerformancePlugin();
        ~SecurityPerformancePlugin() override;

        // Build QueryInterface implementation, specifying all possible interfaces to be returned.
        BEGIN_INTERFACE_MAP(SecurityPerformancePlugin)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(Exchange::ISecurityPerformance)
        END_INTERFACE_MAP

    public:
        //   IPlugin methods
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

        // Exchange::ISecurityPerformance methods
        Exchange::ISecurityPerformance::IForwarder* GetInterface() override;
        Core::hresult Sum(const uint32_t a, const uint32_t b, uint32_t& sum) const override;

        uint32_t Send(const uint16_t sendSize, const uint8_t buffer[]) override;
        uint32_t Receive(uint16_t& bufferSize, uint8_t buffer[]) const override;
        uint32_t Exchange(uint16_t& bufferSize, uint8_t buffer[], const uint16_t maxBufferSize) override;

    private:
        COMServer* _rpcServer;
        Forwarder* _forwarder;
    };

} // namespace Plugin
} // namespace Thunder

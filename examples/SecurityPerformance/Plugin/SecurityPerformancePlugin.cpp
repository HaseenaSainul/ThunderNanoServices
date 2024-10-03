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
 
#include "SecurityPerformancePlugin.h"

namespace Thunder {

namespace Plugin {

    namespace {

        static Metadata<SecurityPerformancePlugin> metadata(
            // Version
            1, 0, 0,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    PUSH_WARNING(DISABLE_WARNING_THIS_IN_MEMBER_INITIALIZER_LIST)
    SecurityPerformancePlugin::SecurityPerformancePlugin()
        : _rpcServer(nullptr)
    {
    }
    POP_WARNING()
    /* virtual */ SecurityPerformancePlugin::~SecurityPerformancePlugin()
    {
    }

    /* virtual */ const string SecurityPerformancePlugin::Initialize(PluginHost::IShell * service)
    {
        Config config;
        config.FromString(service->ConfigLine());

        Core::NodeId source(config.Connector.Value().c_str());
        Core::ProxyType<RPC::InvokeServer> engine (Core::ProxyType<RPC::InvokeServer>::Create(&Core::IWorkerPool::Instance()));
        _rpcServer = new COMServer(Core::NodeId(source, source.PortNumber()), this, service->ProxyStubPath(), engine);

        // On success return empty, to indicate there is no error text.
        return (string());
    }

    /* virtual */ void SecurityPerformancePlugin::Deinitialize(PluginHost::IShell * /* service */)
    {
        delete _rpcServer;
    }

    /* virtual */ string SecurityPerformancePlugin::Information() const
    {
        // No additional info to report.
        return (string());
    }

    Core::hresult SecurityPerformancePlugin::Forwarder::Sum(const uint32_t a, const uint32_t b, uint32_t& sum) const /* override */
    {
         sum = a + b;
         return Core::ERROR_NONE;
    }

    // Exchange::ISecurityPerformance methods
    Exchange::ISecurityPerformance::IForwarder* SecurityPerformancePlugin::GetInterface() /* override */
    {
        return nullptr;
    }
    Core::hresult SecurityPerformancePlugin::Sum(const uint32_t a, const uint32_t b, uint32_t& sum) const /* override */
    {
         sum = a + b;
         return Core::ERROR_NONE;
    }

    uint32_t SecurityPerformancePlugin::Send(const uint16_t sendSize, const uint8_t buffer[] VARIABLE_IS_NOT_USED) /* override */
    {
        uint32_t result = sendSize;
        return (result);
    }
    uint32_t SecurityPerformancePlugin::Receive(uint16_t & bufferSize, uint8_t buffer[]) const /* override */ 
    {
        static uint8_t pattern[] = { 0x00, 0x66, 0xBB, 0xEE };
        uint32_t result = Core::ERROR_NONE;
        uint8_t patternLength = sizeof(pattern);
        uint16_t index = 0;
        uint8_t patternIndex = 0;

        while (index < bufferSize) {

            buffer[index++] = pattern[patternIndex++];

            patternIndex %= (patternLength - 1);
        }

        return (result);
    }
    uint32_t SecurityPerformancePlugin::Exchange(uint16_t & bufferSize VARIABLE_IS_NOT_USED, uint8_t buffer[], const uint16_t maxBufferSize) /* override */
    {
        uint32_t result = Core::ERROR_NONE;
        static uint8_t pattern[] = { 0x00, 0x77, 0xCC, 0x88 };
        uint8_t patternLength = sizeof(pattern);
        uint16_t index = 0;
        uint8_t patternIndex = 0;

        while (index < maxBufferSize) {

            buffer[index++] = pattern[patternIndex++];

            patternIndex %= (patternLength - 1);
        }

        return (result);
    }

} // namespace Plugin

} // namespace Thunder

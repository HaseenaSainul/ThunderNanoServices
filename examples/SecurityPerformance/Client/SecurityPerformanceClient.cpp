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
 
#include "Module.h"

#include <interfaces/ISecurityPerformance.h>

using namespace Thunder;

bool ParseOptions(int argc, char** argv, Core::NodeId& comChannel)
{
    int index = 1;
    const char* hostname = _T("127.0.0.1:8899");
    bool showHelp = false;

    while ((index < argc) && (!showHelp)) {
        if (strcmp(argv[index], "-remote") == 0) {
            hostname = argv[index + 1];
            index++;
        } else if (strcmp(argv[index], "-h") == 0) {
            showHelp = true;
        }
        index++;
    }

    if (!showHelp) {
        comChannel = Core::NodeId(hostname);
    }

    return (showHelp);
}

void ShowMenu()
{
    printf("Enter\n"
           "\tD : Measure performance of send/recieve/exchange\n"
           "\tF : Measure performance of forwarder interfaces\n"
           "\tS : Measure performance of interface:sum()\n"
           "\tH : Help\n"
           "\tQ : Quit\n");
}

void ShowPerformanceMenu()
{
    printf("Enter\n"
           "\tS : Test sending data\n"
           "\tR : Test receiving data\n"
           "\tE : Test exchanging data\n"
           "\tQ : Quit\n");
}

// Performance measurement functions/methods and definitions
// ---------------------------------------------------------------------------------------------
typedef std::function<uint32_t(uint16_t& size, uint8_t buffer[])> PerformanceFunction;

constexpr uint32_t MeasurementLoops = 20;
static uint8_t swapPattern[] = { 0x00, 0x55, 0xAA, 0xFF };

static void Measure(const uint8_t patternLength, const uint8_t pattern[], PerformanceFunction& subject)
{
    uint8_t dataFrame[1024 * 10];
    uint16_t index = 0;
    uint8_t patternIndex = 0;

    ASSERT(patternLength != 0);

    while (index < sizeof(dataFrame)) {

        dataFrame[index++] = pattern[patternIndex++];

        patternIndex %= (patternLength - 1);
    }

    printf("Measurements :\n");
    uint64_t time;
    Core::StopWatch measurement;
    uint16_t length = 0;

    for (uint32_t run = 0; run < MeasurementLoops; run++) {
        subject(length, dataFrame);
    }
    time = measurement.Elapsed();
    printf("Data outbound:    [0], inbound:    [4]. Total: %" PRIu64 ". Average: %" PRIu64 "\n", time, time / MeasurementLoops);

    measurement.Reset();
    length = 16;
    for (uint32_t run = 0; run < MeasurementLoops; run++)
    {
        subject(length, dataFrame);
    }
    time = measurement.Elapsed();
    printf("Data outbound:   [16], inbound:    [4]. Total: %" PRIu64 ". Average: %" PRIu64 "\n", time, time / MeasurementLoops);

    measurement.Reset();
    length = 128;
    for (uint32_t run = 0; run < MeasurementLoops; run++)
    {
        subject(length, dataFrame);
    }
    time = measurement.Elapsed();
    printf("Data outbound:  [128], inbound:    [4]. Total: %" PRIu64 ". Average: %" PRIu64 "\n", time, time / MeasurementLoops);

    measurement.Reset();
    length = 256;
    for (uint32_t run = 0; run < MeasurementLoops; run++) {
        subject(length, dataFrame);
    }
    time = measurement.Elapsed();
    printf("Data outbound:  [256], inbound:    [4]. Total: %" PRIu64 ". Average: %" PRIu64 "\n", time, time / MeasurementLoops);

    measurement.Reset();
    length = 512;
    for (uint32_t run = 0; run < MeasurementLoops; run++) {
        subject(length, dataFrame);
    }
    time = measurement.Elapsed();
    printf("Data outbound:  [512], inbound:    [4]. Total: %" PRIu64 ". Average: %" PRIu64 "\n", time, time / MeasurementLoops);

    measurement.Reset();
    length = 1024;
    for (uint32_t run = 0; run < MeasurementLoops; run++) {
        subject(length, dataFrame);
    }
    time = measurement.Elapsed();;
    printf("Data outbound: [1024], inbound:    [4]. Total: %" PRIu64 ". Average: %" PRIu64 "\n", time, time / MeasurementLoops);

    measurement.Reset();
    length = 2048;
    for (uint32_t run = 0; run < MeasurementLoops; run++) {
        subject(length, dataFrame);
    }
    time = measurement.Elapsed();
    printf("Data outbound: [2048], inbound:    [4]. Total: %" PRIu64 ". Average: %" PRIu64 "\n", time, time / MeasurementLoops);

    measurement.Reset();
    length = (1024 * 8) - 1;
    for (uint32_t run = 0; run < MeasurementLoops; run++) {
        subject(length, dataFrame);
    }
    time = measurement.Elapsed();
    printf("Data outbound: [8KB], inbound:    [4]. Total: %" PRIu64 ". Average: %" PRIu64 "\n", time, time / MeasurementLoops);

    measurement.Reset();
    length = (1024 * 8) + 100;
    for (uint32_t run = 0; run < MeasurementLoops; run++) {
        subject(length, dataFrame);
    }
    time = measurement.Elapsed();
    printf("Data outbound: [8KB + 100], inbound:    [4]. Total: %" PRIu64 ". Average: %" PRIu64 "\n", time, time / MeasurementLoops);

}

void MeasureDataFlow(Core::ProxyType<RPC::CommunicatorClient>& client)
{
    if ((client.IsValid() == false) || (client->IsOpen() == false)) {
        printf("Can not measure the performance of COMRPC, there is no connection.\n");
    } else {
        Core::StopWatch measurement;
        Exchange::ISecurityPerformance* perf = client->Acquire<Exchange::ISecurityPerformance>(2000, _T("SecurityPerformancePlugin"), ~0);
        if (perf == nullptr) {
            printf("Instantiation failed. An performance interface was not returned. It took: %" PRIu64 " ticks\n", measurement.Elapsed());
        } else {
            printf("Instantiating and retrieving the interface took: %" PRIu64 " ticks\n", measurement.Elapsed());
            int measure;
            do {
                ShowPerformanceMenu();
                getchar(); // Skip white space
                measure = toupper(getchar());
                switch (measure) {
                case 'S': {
                    PerformanceFunction implementation = [perf](const uint16_t length, const uint8_t buffer[]) -> uint32_t {
                        return (perf->Send(length, buffer));
                    };

                    Measure(sizeof(swapPattern), swapPattern, implementation);
                    break;
                }
                case 'R': {
                    PerformanceFunction implementation = [perf](uint16_t& length, uint8_t buffer[]) -> uint32_t {
                        return (perf->Receive(length, buffer));
                    };
                    Measure(sizeof(swapPattern), swapPattern, implementation);
                    break;
                }
                case 'E': {
                    PerformanceFunction implementation = [perf](uint16_t& length, uint8_t buffer[]) -> uint32_t {
                        const uint16_t maxBufferSize = length;
                        return (perf->Exchange(length, buffer, maxBufferSize));
                    };
                    Measure(sizeof(swapPattern), swapPattern, implementation);
                    break;
                }
                default: {
                    break;
                }
                }
            } while (measure != 'Q');
            perf->Release();
        }
    }
}

typedef std::function<uint32_t(const uint32_t a, const uint32_t b, uint32_t& sum)> SumFunction;
void Measure(const TCHAR info[], SumFunction& sum)
{
    Core::StopWatch measurement;
    const uint32_t a = 8888, b = 1111;
    uint32_t s = 0;

    uint64_t time;
    for (uint32_t run = 0; run < MeasurementLoops; run++) {
        sum(a, b, s);
        printf("sum = %d\n", s);
    }
    printf("Measurements [%s]:\n", info);
    time = measurement.Elapsed();
    printf("Total time : %" PRIu64 " Average time: %" PRIu64 " \n", time, time / MeasurementLoops);
}
void MeasureSum(Core::ProxyType<RPC::CommunicatorClient>& client, bool isForwarder)
{
    if ((client.IsValid() == false) || (client->IsOpen() == false)) {
        printf("Can not measure the performance of COMRPC, there is no connection.\n");
    } else {
        Exchange::ISecurityPerformance* perf = client->Acquire<Exchange::ISecurityPerformance>(2000, _T("SecurityPerformancePlugin"), ~0);
        if (perf == nullptr) {
            printf("Instantiation failed. An performance interface was not returned\n");
        } else {
            if (isForwarder) {
                Exchange::ISecurityPerformance::IForwarder* forwarder = perf->GetInterface();
                if (forwarder != nullptr) {
                    SumFunction implementation = [forwarder](const uint32_t a, const uint32_t b, uint32_t& sum) -> uint32_t {
                        return forwarder->Sum(a, b, sum);
                    };
                    Measure(_T("Forwarder:Sum"), implementation);
                }
            } else {
                printf("Measurements [Sum]: ");
                SumFunction implementation = [perf](const uint32_t a, const uint32_t b, uint32_t& sum) -> Core::hresult {
                    return perf->Sum(a, b, sum);
                };
                Measure(_T("Sum"), implementation);
            }
        }
    }
}

int main(int argc, char** argv)
{
    // Additional scoping neede to have a proper shutdown of the STACK object:
    // JSONRPC::LinkType<Core::JSON::IElement> remoteObject
    {

        Core::NodeId comChannel;
        ShowMenu();
        int element;

        ParseOptions(argc, argv, comChannel);

        // If others are started at the same time (from Visual Studio :-) give the server a bit more time to start.
        SleepMs(4000);

        // Lets also open up channels over the COMRPC protocol to do performance measurements
        // to compare JSONRPC v.s. COMRPC

        // Make sure we have an engine to handle the incoming requests over the COMRPC channel.
        // They are time multiplexed so 1 engine to rule them all. The next line instantiates the
        // framework to connect to a COMRPC server running at the <connector> address. once the
        // connection is established, interfaces can be requested.
        Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> engine(Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create());
        Core::ProxyType<RPC::CommunicatorClient> client (
            Core::ProxyType<RPC::CommunicatorClient>::Create(
                comChannel, 
                Core::ProxyType<Core::IIPCServer>(engine)
            ));

        ASSERT(client.IsValid() == true);

        // Open up the COMRPC Client connection.
        if (client->Open(2000) != Core::ERROR_NONE) {
            printf("Failed to open up a COMRPC link with the server. Is the server running ?\n");
        }

        do {
            printf("\n>");
            element = toupper(getchar());

            switch (element) {
            case 'F':
            {
                MeasureSum(client, true);
                break;
            }
            case 'S':
            {
                MeasureSum(client, false);
                break;
            }
            case 'D':
            {
                MeasureDataFlow(client);
                break;
            }
            case '?':
            case 'H':
                ShowMenu();
            }

        } while (element != 'Q');

        // We are done with the COMRPC connections, no need to create new ones.
        client->Close(Core::infinite);
        client.Release();
    }

    printf("Leaving app.\n");

    Core::Singleton::Dispose();

    return (0);
}

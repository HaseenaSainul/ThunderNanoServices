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
#include <interfaces/json/JsonData_Browser.h>
#include "Cobalt.h"

namespace Thunder {
namespace Plugin {

using namespace JsonData::Browser;

// Registration
//
void Cobalt::RegisterAll()
{
    Property < Core::JSON::String
            > (_T("url"), &Cobalt::get_url, &Cobalt::set_url, this); /* Browser */
    Property < Core::JSON::EnumType
            < VisibilityType
                    >> (_T("visibility"), &Cobalt::get_visibility, &Cobalt::set_visibility, this); /* Browser */
    Property < Core::JSON::DecUInt32
            > (_T("fps"), &Cobalt::get_fps, nullptr, this); /* Browser */
    Register<DeleteParamsData,void>(_T("delete"), &Cobalt::endpoint_delete, this);
    Property < Core::JSON::String
            > (_T("deeplink"), nullptr, &Cobalt::set_deeplink, this); /* Application */
}

void Cobalt::UnregisterAll()
{
    Unregister(_T("fps"));
    Unregister(_T("visibility"));
    Unregister(_T("url"));
    Unregister(_T("delete"));
    Unregister(_T("deeplink"));
}

// API implementation
//

// Property: url - URL loaded in the browser
// Return codes:
//  - ERROR_NONE: Success
uint32_t Cobalt::get_url(Core::JSON::String &response) const /* Browser */
{
    ASSERT(_cobalt != nullptr);
    response = _cobalt->GetURL();
    return Core::ERROR_NONE;
}

// Property: url - URL loaded in the browser
// Return codes:
//  - ERROR_NONE: Success
//  - ERROR_INCORRECT_URL: Incorrect URL given
uint32_t Cobalt::set_url(const Core::JSON::String &param) /* Browser */
{
    ASSERT(_cobalt != nullptr);
    uint32_t result = Core::ERROR_INCORRECT_URL;
    if (param.IsSet() && !param.Value().empty()) {
        _cobalt->SetURL(param.Value());
        result = Core::ERROR_NONE;
    }
    return result;
}

// Property: visibility - Current browser visibility
// Return codes:
//  - ERROR_NONE: Success
uint32_t Cobalt::get_visibility(
        VARIABLE_IS_NOT_USED Core::JSON::EnumType<VisibilityType> &response) const /* Browser */
{
    /*
    response = (_hidden ? VisibilityType::HIDDEN : VisibilityType::VISIBLE);
    return Core::ERROR_NONE;
    */
    return Core::ERROR_UNAVAILABLE;
}

// Property: visibility - Current browser visibility
// Return codes:
//  - ERROR_NONE: Success
uint32_t Cobalt::set_visibility(
        VARIABLE_IS_NOT_USED const Core::JSON::EnumType<VisibilityType> &param) /* Browser */
{
    /*
    ASSERT(_cobalt != nullptr);
    uint32_t result = Core::ERROR_BAD_REQUEST;
    if (param.IsSet()) {
        if (param == VisibilityType::VISIBLE) {
            _cobalt->Hide(true);
        } else {
            _cobalt->Hide(false);
        }
        result = Core::ERROR_NONE;
    }
    return result;
    */
    return Core::ERROR_UNAVAILABLE;
}

// Property: fps - Current number of frames per second the browser is rendering
// Return codes:
//  - ERROR_NONE: Success
uint32_t Cobalt::get_fps(Core::JSON::DecUInt32 &response) const /* Browser */
{
    ASSERT(_cobalt != nullptr);
    response = _cobalt->GetFPS();
    return Core::ERROR_NONE;
}

// Method: endpoint_delete - delete dir
// Return codes:
//  - ERROR_NONE: Success
//  - ERROR_UNKNOWN_KEY: The given path was incorrect
uint32_t Cobalt::endpoint_delete(const DeleteParamsData& params)
{
    return DeleteDir(params.Path.Value());
}

// Property: deeplink - ContentLink loaded in the browser
// Return codes:
//  - ERROR_NONE: Success
//  - ERROR_INCORRECT_URL: Incorrect ContentLink given
uint32_t Cobalt::set_deeplink(const Core::JSON::String &param) /* Application */
{
    ASSERT(_application != nullptr);
    uint32_t result = Core::ERROR_INCORRECT_URL;
    if (param.IsSet() && !param.Value().empty()) {
        _application->ContentLink(param.Value());
        result = Core::ERROR_NONE;
    }
    return result;
}

// Event: urlchange - Signals a URL change in the browser
void Cobalt::event_urlchange(const string &url, const bool &loaded) /* Browser */
{
    UrlchangeParamsData params;
    params.Url = url;
    params.Loaded = loaded;

    Notify(_T("urlchange"), params);
}

// Event: visibilitychange - Signals a visibility change of the browser
void Cobalt::event_visibilitychange(const bool &hidden) /* Browser */
{
    VisibilitychangeParamsData params;
    params.Hidden = hidden;

    Notify(_T("visibilitychange"), params);
}

} // namespace Plugin
} // namespace Thunder

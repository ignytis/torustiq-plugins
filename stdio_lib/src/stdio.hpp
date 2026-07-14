#ifndef _TORUSTIQ_PLUGINS_STDIO_LIB_STDIO_H_
#define _TORUSTIQ_PLUGINS_STDIO_LIB_STDIO_H_

#include <torustiq_sdk/typedefs.h>

namespace TorustiqPlugins {
namespace Stdio {

const TorustiqPluginInfo GetPluginInfo();

const TorustiqPlugin InitPlugin(TorustiqHostGlobals globals);

void CreateNewStage(CreateNewStageFnArgs args);

void SetStageConfigValue(TorustiqPluginStageHandle stageHandle, const char* key,
                         const char* value);

void Start(TorustiqPluginStageHandle stageHandle);

}  // namespace Stdio
}  // namespace TorustiqPlugins

#endif

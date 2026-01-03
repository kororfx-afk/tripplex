// Sample MQL5 EA that demonstrates calling the Tripplex DLL exported C functions.
// This is a minimal example and not a production EA.

#property script_show_inputs

import "tripplex_dll_api.dll"

extern string config_json = "{\"endpoint\": \"127.0.0.1:9000\"}";

int OnInit()
{
  // TPX_Init and TPX_Subscribe are C functions exported from the DLL.
  int res = TPX_Init(config_json);
  if(res != 0)
  {
    Print("TPX_Init failed: ", res);
    return INIT_FAILED;
  }

  TPX_Subscribe("EURUSD", 1);
  return INIT_SUCCEEDED;
}

void OnDeinit(const int reason)
{
  TPX_Shutdown();
}

void OnTick()
{
  // In real EA, ticks from terminal would be pushed to DLL which forwards to Tripplex Core.
}

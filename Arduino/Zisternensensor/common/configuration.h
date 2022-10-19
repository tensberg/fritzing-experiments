const uint8_t ap_wifi_channel = 1;

typedef struct {
  double temperature;
  double pressure;
  double humidity;
  int distance;
} ZisternensensorData;


typedef struct {
  unsigned long iteration;
  unsigned long sleepTimeS;
  unsigned long activeTimeS;
  unsigned int activeTimeMicros;
  unsigned int sendSuccessCount;
  unsigned int sendFailedCount;
  int lastSendStatus;
  int lastSendFailedStatus;
} ZisternensensorStatus;

typedef struct {
  ZisternensensorData data;
  ZisternensensorStatus status;
} ZisternensensorMessage;

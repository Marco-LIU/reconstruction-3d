#pragma  once

// 是否启用pumping的profile，用来测试性能
// 日志输出到debug.log中
// 推荐在release模式使用
#ifdef DEBUG
#define ENABLE_CAMERA_PUMP_PROFILE 0
#else
#define ENABLE_CAMERA_PUMP_PROFILE 1
#endif // DEBUG
